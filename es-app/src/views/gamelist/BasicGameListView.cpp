#include "views/gamelist/BasicGameListView.h"
#include "views/ViewController.h"
#include "Renderer.h"
#include "Window.h"
#include "ThemeData.h"
#include "SystemData.h"
#include "Settings.h"
#include "SystemManager.h"

BasicGameListView::BasicGameListView(Window* window, const FileData& root)
	: ISimpleGameListView(window, root), mList(window)
{
	mList.setSize(mSize.x(), mSize.y() * 0.8f);
	mList.setPosition(0, mSize.y() * 0.2f);
	addChild(&mList);

	populateList(mCursorStack.top().getChildren());
}

void BasicGameListView::onThemeChanged(const std::shared_ptr<ThemeData>& theme)
{
	ISimpleGameListView::onThemeChanged(theme);
	using namespace ThemeFlags;
	mList.applyTheme(theme, getName(), "gamelist", ALL);
}

void BasicGameListView::onFilesChanged()
{
	// might switch to a detailed view
	ViewController::get()->reloadGameListView(this);
}

void BasicGameListView::onMetaDataChanged(const FileData& file)
{
	// might switch to a detailed view
	ViewController::get()->reloadGameListView(this);
}

void BasicGameListView::populateList(const std::vector<FileData>& files)
{
	mList.clear();

	mHeaderText.setText(mRoot.getSystem()->getFullName());

	for(auto it = files.begin(); it != files.end(); it++)
	{
		mList.add(it->getName(), *it, (it->getType() == FOLDER));
	}
}

const FileData& BasicGameListView::getCursor()
{
	return mList.getSelected();
}

void BasicGameListView::setCursor(const FileData& cursor)
{
	if(!mList.setCursor(cursor))
	{
		LOG(LogWarning) << "Tried to setCursor to non-present filedata (" << cursor.getFileID() << " for system " << cursor.getSystemID() << ")";
	}
}

void BasicGameListView::launch(FileData& game)
{
	ViewController::get()->launch(game);
}

void BasicGameListView::remove(const FileData &game)
{

	boost::filesystem::remove(game.getPath());  // actually delete the file on the filesystem
	if (getCursor() == game)                     // Select next element in list, or prev if none
	{
		std::vector<FileData> siblings = game.getChildren();
		auto gameIter = std::find(siblings.begin(), siblings.end(), game);
		auto gamePos = std::distance(siblings.begin(), gameIter);
		if (gameIter != siblings.end())
		{
			if ((gamePos + 1) < siblings.size())
			{
				setCursor(siblings.at(gamePos + 1));
			} else if ((gamePos - 1) > 0) {
				setCursor(siblings.at(gamePos - 1));
			}
		}
	}
	// remove before updating
	SystemManager::getInstance()->database().updateExists(game);
	onFilesChanged();
}

std::vector<HelpPrompt> BasicGameListView::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;

	if(Settings::getInstance()->getBool("QuickSystemSelect"))
		prompts.push_back(HelpPrompt("left/right", "system"));
	prompts.push_back(HelpPrompt("up/down", "choose"));
	prompts.push_back(HelpPrompt("a", "launch"));
	prompts.push_back(HelpPrompt("b", "back"));
	prompts.push_back(HelpPrompt("select", "options"));
	return prompts;
}
