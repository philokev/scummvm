/*
 * dialogs.h
 *
 *  Created on: 21 May 2020
 *      Author: kevin
 */

#ifndef ENGINES_YAHTZEE_DIALOGS_H_
#define ENGINES_YAHTZEE_DIALOGS_H_

#include "yahtzee/yahtzee.h"
#include "gui/dialog.h"
#include "gui/ThemeEngine.h"
#include "gui/widgets/edittext.h"
#include "gui/widget.h"
#include "common/array.h"
#include "common/ptr.h"

#include "yahtzee/gui/widget.h"
#include "yahtzee/config.h"

namespace Image {
class ImageDecoder;
}

namespace Yahtzee {

enum Options {
	// Main menu
	kCmdNewGame = 'NEWG',
	kCmdContinue = 'CONT',
	kCmdPassword = 'PASS',
	kCmdQuit = 'QUIT'
};

enum IntroOptions {
	kWatchIntro = 'INTR',
	kSkipIntro = 'SKIP'
};

const Common::String INTRO_FORM = "Form1";
const Common::String DEATH_FORM = "Form9";
const Common::String PASSWORD_FORM = "Form17";

const Common::String INTRO_BACKGROUND_IMAGE = "Image1";

const Common::String FIRST_PARAGRAPH = "Label1";
const Common::String SECOND_PARAGRAPH = "Label2";
const Common::String THIRD_PARAGRAPH = "Label3";
const Common::String FOURTH_PARAGRAPH = "Label4";

const Common::String INTRO_PARAGRAPH_FONT_FILE_NAME = MS_SERIF_BOLD_FONT_FILE_NAME;

const Common::String SCARED_BUTTON = "Command1";

const Common::String MAIN_MENU_BUTTON_FONT_FILE_NAME = MS_TIMES_NEW_ROMAN_BOLD_ITALIC_FONT_FILE_NAME;

const Common::String NEW_GAME_LABEL = "Label9";
const Common::String QUIT_LABEL = "Label10";
const Common::String PASSWORD_LABEL = "Label11";

const Common::String WATCH_INTRO_OPTION = "Option1";
const Common::String SKIP_INTRO_OPTION = "Option2";

const Common::String ENTER_PASSWORD_LABEL = "Label1";

const Common::String MAIN_MENU_PASSWORD_FONT_FILE_NAME = MS_SANS_SERIF_BOLD_FONT_FILE_NAME;
const Common::String MAIN_MENU_PASSWORD_FONT_NAME = MS_SANS_SERIF_FONT_NAME;

const Common::String PASSWORD_TEXTBOX = "Text1";
const Common::String PASSWORD_FONT_FILE_NAME = MS_SANS_SERIF_BOLD_FONT_FILE_NAME;
const Common::String PASSWORD_FONT_NAME = MS_SANS_SERIF_FONT_NAME;
const int PASSWORD_FONT_SIZE = 8;

const Common::String PASSWORD_OK_BUTTON = "Command2";

const Common::String DEATH_IMAGE = "Image1";
const Common::String DEATH_QUIT_BUTTON = "Command1";


const Common::String PASSWORDS[] = {
		"",
		"CHOP SUEY",
		"LUBRICANT",
		"STRESSED",
		"INFIDEL",
		"SNIFFER",
		"UNTOWARD",
		"CLIMAX"
};




using FormPtr = Common::ScopedPtr<Parser::Form>;
using LabelPtr = Common::ScopedPtr<Parser::Label>;
using CommandButtonPtr = Common::ScopedPtr<Parser::CommandButton>;
using OptionButtonPtr = Common::ScopedPtr<Parser::OptionButton>;
using TextBoxPtr = Common::ScopedPtr<Parser::TextBox>;


using FontPtr = Common::ScopedPtr<Graphics::Font>;
using ContainerWidgetPtr = Common::ScopedPtr<GUI::ContainerWidget>;
using GraphicsWidgetPtr = Common::ScopedPtr<GUI::GraphicsWidget>;
using ButtonWidgetPtr = Common::ScopedPtr<GUI::ButtonWidget>;
using RadioButtonGroupPtr = Common::ScopedPtr<GUI::RadiobuttonGroup>;
using RadiobuttonWidgetPtr = Common::ScopedPtr<GUI::RadiobuttonWidget>;

using GraphicsWidgetPtrArray = Common::Array<GraphicsWidgetPtr>;
using RadiobuttonWidgetPtrArray = Common::Array<RadiobuttonWidgetPtr>;
using SurfacePtr = Common::ScopedPtr<Graphics::Surface, Graphics::SurfaceDeleter>;

class YahtzeeDialog : public GUI::Dialog {
public:
	YahtzeeDialog(YahtzeeGame*);
	~YahtzeeDialog();
protected:
	YahtzeeGame* _vm;
};

class MainMenu : public YahtzeeDialog {
public:
	MainMenu(YahtzeeGame*);
	~MainMenu() override;

	void reflowLayout() override;
	void handleCommand(GUI::CommandSender*, uint32, uint32) override;
	void open() override;

private:
	void setBackground();
	void loadIntroScreen();
	void unloadIntroScreen();
	void loadPasswordButton();
	void loadMainMenu();
	GUI::GraphicsWidget* buildLabel(const Parser::Label&, const Graphics::Font&);
	ClickableImageWidget* buildClickableLabel(const Parser::Label&, const Graphics::Font&, uint32 cmd = 0);

	FormPtr _introForm;
	GraphicsWidgetPtrArray _labels;
	ClickableImageWidgetPtr _password;
	ClickableImageWidgetPtr _startGame;
	ClickableImageWidgetPtr _quitGame;
	ButtonWidgetPtr _scared;
	GraphicsWidgetPtr _background;
	RadioButtonGroupPtr _options;
	RadiobuttonWidgetPtrArray _choices;
};

class BottomMenu : public YahtzeeDialog {
public:
	BottomMenu(Engine*);
	~BottomMenu() override;
};

class PasswordDialog : public YahtzeeDialog {
public:
	PasswordDialog(YahtzeeGame*);
	~PasswordDialog() override;

	void reflowLayout() override;
	void handleCommand(GUI::CommandSender*, uint32, uint32) override;
protected:
	FormPtr _passwordForm;
	ContainerImageWidgetPtr _container;
	GraphicsWidgetPtr _passwordLabel;
	PasswordTextWidgetPtr _password;
	ButtonWidgetPtr _ok;
private:
	GUI::GraphicsWidget* buildLabelWithBackgroundColor(const Parser::Label&, const Graphics::Font&, uint32 color);
};

class DeathDialog : public YahtzeeDialog {
public:
	DeathDialog(YahtzeeGame*);
	~DeathDialog() override;

	void reflowLayout() override;
	void handleCommand(GUI::CommandSender*, uint32, uint32) override;
protected:
	FormPtr _deathForm;
	GraphicsWidgetPtr _hat;
	ButtonWidgetPtr _quit;
private:
	void setBackground();
};


}



#endif /* ENGINES_YAHTZEE_DIALOGS_H_ */
