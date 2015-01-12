#include "GuiComponent.h"
#include "components/MenuComponent.h"
#include "components/SwitchComponent.h"
#include "components/OptionListComponent.h"
#include "FileSorts.h"

class IGameListView;

class GuiGamelistOptions : public GuiComponent
{
public:
	GuiGamelistOptions(Window* window, SystemData* system);
	virtual ~GuiGamelistOptions();

	virtual bool input(InputConfig* config, Input input) override;
	virtual std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void openMetaDataEd();
	void jumpToLetter();
	
	MenuComponent mMenu;

	typedef OptionListComponent<char> LetterList;
	std::shared_ptr<LetterList> mJumpToLetterList;

	typedef OptionListComponent<const FileData::SortType*> SortList;
	std::shared_ptr<SortList> mListSort;

	typedef SwitchComponent DeleteSwitch;
	std::shared_ptr<DeleteSwitch> mFastDel;
	
	typedef SwitchComponent FileSwitch;
	std::shared_ptr<FileSwitch> mShowFile;

	SystemData* mSystem;
	IGameListView* getGamelist();
};
