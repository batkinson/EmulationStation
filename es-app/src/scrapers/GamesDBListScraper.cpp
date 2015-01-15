#include "scrapers/GamesDBListScraper.h"
#include "Log.h"
#include "pugixml/pugixml.hpp"
#include "MetaData.h"
#include "Settings.h"
#include "Util.h"
#include <boost/assign.hpp>

using namespace PlatformIds;
const std::map<PlatformId, const char*> gamesdb_platformid_map = boost::assign::map_list_of
	(THREEDO, "3DO")
	(AMIGA, "Amiga")
	(AMSTRAD_CPC, "Amstrad CPC")
	// missing apple2
	(ARCADE, "Arcade")
	// missing atari 800
	(ATARI_2600, "Atari 2600")
	(ATARI_5200, "Atari 5200")
	(ATARI_7800, "Atari 7800")
	(ATARI_JAGUAR, "Atari Jaguar")
	(ATARI_JAGUAR_CD, "Atari Jaguar CD")
	(ATARI_LYNX, "Atari Lynx")
	// missing atari ST/STE/Falcon
	(ATARI_XE, "Atari XE")
	(COLECOVISION, "Colecovision")
	(COMMODORE_64, "Commodore 64")
	(INTELLIVISION, "Intellivision")
	(MAC_OS, "Mac OS")
	(XBOX, "Microsoft Xbox")
	(XBOX_360, "Microsoft Xbox 360")
	(NEOGEO, "NeoGeo")
	(NEOGEO_POCKET, "Neo Geo Pocket")
	(NEOGEO_POCKET_COLOR, "Neo Geo Pocket Color")
	(NINTENDO_3DS, "Nintendo 3DS")
	(NINTENDO_64, "Nintendo 64")
	(NINTENDO_DS, "Nintendo DS")
	(NINTENDO_ENTERTAINMENT_SYSTEM, "Nintendo Entertainment System (NES)")
	(GAME_BOY, "Nintendo Game Boy")
	(GAME_BOY_ADVANCE, "Nintendo Game Boy Advance")
	(GAME_BOY_COLOR, "Nintendo Game Boy Color")
	(NINTENDO_GAMECUBE, "Nintendo GameCube")
	(NINTENDO_WII, "Nintendo Wii")
	(NINTENDO_WII_U, "Nintendo Wii U")
	(PC, "PC")
	(SEGA_32X, "Sega 32X")
	(SEGA_CD, "Sega CD")
	(SEGA_DREAMCAST, "Sega Dreamcast")
	(SEGA_GAME_GEAR, "Sega Game Gear")
	(SEGA_GENESIS, "Sega Genesis")
	(SEGA_MASTER_SYSTEM, "Sega Master System")
	(SEGA_MEGA_DRIVE, "Sega Mega Drive")
	(SEGA_SATURN, "Sega Saturn")
	(PLAYSTATION, "Sony Playstation")
	(PLAYSTATION_2, "Sony Playstation 2")
	(PLAYSTATION_3, "Sony Playstation 3")
	(PLAYSTATION_4, "Sony Playstation 4")
	(PLAYSTATION_VITA, "Sony Playstation Vita")
	(PLAYSTATION_PORTABLE, "Sony PSP")
	(SUPER_NINTENDO, "Super Nintendo (SNES)")
	(TURBOGRAFX_16, "TurboGrafx 16")
	(WONDERSWAN, "WonderSwan")
	(WONDERSWAN_COLOR, "WonderSwan Color")
	(ZX_SPECTRUM, "Sinclair ZX Spectrum");


void thegamesdblist_generate_scraper_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests,
	std::vector<ScraperSearchResult>& results)
{
	std::string path = "thegamesdb.net/api/GetGamesList.php?";

	std::string cleanName = params.nameOverride;
	if(cleanName.empty())
		cleanName = params.game.getCleanName();

	path += "name=" + HttpReq::urlEncode(cleanName);

	if(params.system->getPlatformIds().empty())
	{
		// no platform specified, we're done
		requests.push(std::unique_ptr<ScraperRequest>(new TheGamesDBListRequest(results, path)));
	}else{
		// go through the list, we need to split this into multiple requests
		// because TheGamesDB API either sucks or I don't know how to use it properly...
		std::string urlBase = path;
		auto& platforms = params.system->getPlatformIds();
		for(auto platformIt = platforms.begin(); platformIt != platforms.end(); platformIt++)
		{
			path = urlBase;
			auto mapIt = gamesdb_platformid_map.find(*platformIt);
			if(mapIt != gamesdb_platformid_map.end())
			{
				path += "&platform=";
				path += HttpReq::urlEncode(mapIt->second);
			}else{
				LOG(LogWarning) << "TheGamesDBList scraper warning - no support for platform " << getPlatformName(*platformIt);
			}

			requests.push(std::unique_ptr<ScraperRequest>(new TheGamesDBListRequest(results, path)));
		}
	}
}

void TheGamesDBListRequest::process(const std::unique_ptr<HttpReq>& req, std::vector<ScraperSearchResult>& results)
{
	assert(req->status() == HttpReq::REQ_SUCCESS);

	pugi::xml_document listDoc;
	pugi::xml_parse_result listParseResult = listDoc.load(req->getContent().c_str());

	if(!listParseResult)
	{
		std::stringstream ss;
		ss << "GamesDBListRequest - Error parsing GamesList XML. \n\t" << listParseResult.description() << "";
		std::string err = ss.str();
		setError(err);
		LOG(LogError) << err;
		return;
	}

	pugi::xml_node listData = listDoc.child("Data");
	pugi::xml_node game = listData.child("Game");

	while(game && results.size() < MAX_SCRAPER_RESULTS)
	{
		ScraperSearchResult result;
		std::string gameId = game.child("id").text().get();
		std::string gameUrl = "thegamesdb.net/api/GetGame.php?id=" + gameId;

		// Perform GetGame request for each game id in GetGamesList result
		HttpReq gameReq(gameUrl);

		// Wait for request to finish (make it synchronous)
		while (gameReq.status() == HttpReq::REQ_IN_PROGRESS);

		if (gameReq.status() == HttpReq::REQ_SUCCESS)
		{
			// Process response into game result, results are unique based on id
			pugi::xml_document doc;
			pugi::xml_parse_result parseResult = doc.load(gameReq.getContent().c_str());
			if(!parseResult)
			{
				std::stringstream ss;
				ss << "GamesDBList HttpReq - Error parsing Game XML. \n\t" << parseResult.description() << "";
				std::string err = ss.str();
				setError(err);
				LOG(LogError) << err;
			}
			else
			{
				pugi::xml_node data = doc.child("Data");
				std::string baseImageUrl = data.child("baseImgUrl").text().get();
				pugi::xml_node game = data.child("Game");

				result.metadata.set("name", game.child("GameTitle").text().get());
				result.metadata.set("desc", game.child("Overview").text().get());

				boost::posix_time::ptime rd = string_to_ptime(game.child("ReleaseDate").text().get(), "%m/%d/%Y");
				result.metadata.set("releasedate", rd);

				result.metadata.set("developer", game.child("Developer").text().get());
				result.metadata.set("publisher", game.child("Publisher").text().get());
				result.metadata.set("genre", game.child("Genres").first_child().text().get());
				result.metadata.set("players", game.child("Players").text().get());

				if(Settings::getInstance()->getBool("ScrapeRatings") && game.child("Rating"))
				{
					float ratingVal = (game.child("Rating").text().as_int() / 10.0f);
					result.metadata.set("rating", ratingVal);
				}

				pugi::xml_node images = game.child("Images");

				if(images)
				{
					pugi::xml_node art = images.find_child_by_attribute("boxart", "side", "front");
					if(art)
					{
						result.thumbnailUrl = baseImageUrl + art.attribute("thumb").as_string();
						result.imageUrl = baseImageUrl + art.text().get();
					}
				}
				results.push_back(result);
			}
		}
		game = game.next_sibling("Game");
	}
}
