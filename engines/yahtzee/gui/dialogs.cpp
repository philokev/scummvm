/*
 * dialogs.cpp
 *
 *  Created on: 21 May 2020
 *      Author: kevin
 */

#include "common/file.h"
#include "common/hashmap.h"
#include "common/ptr.h"
#include "common/str.h"
#include "common/str-array.h"
#include "common/substream.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "gui/dialog.h"
#include "gui/gui-manager.h"
#include "gui/ThemeEngine.h"
#include "gui/widgets/edittext.h"
#include "gui/widget.h"

#include "image/bmp.h"

#include "yahtzee/gui/dialogs.h"
#include "yahtzee/controls.h"
#include "yahtzee/yahtzee.h"
#include "yahtzee/util.h"

namespace Yahtzee {

YahtzeeDialog::YahtzeeDialog(YahtzeeGame* vm) : GUI::Dialog(0, 0, 0, 0), _vm(vm) {}

YahtzeeDialog::~YahtzeeDialog() {}

MainMenu::MainMenu(YahtzeeGame* vm) : YahtzeeDialog(vm), _labels(4), _choices(2)  {
	GUI::Dialog::_backgroundType = GUI::ThemeEngine::kDialogBackgroundMain;

	_introForm.reset(_vm->forms[INTRO_FORM]);
	_x = _introForm->clientLeft;
	_y = _introForm->clientTop;
	_w = _introForm->clientWidth;
	_h = _introForm->clientHeight;
}

MainMenu::~MainMenu() {}

void MainMenu::loadIntroScreen() {
	const auto labels = _introForm->labels;

	const Common::StringArray labelNames = {
			FIRST_PARAGRAPH,
			SECOND_PARAGRAPH,
			THIRD_PARAGRAPH,
			FOURTH_PARAGRAPH
	};

	const auto paragraphLabel = *labels[labelNames[0]];
	auto& font = *buildWinFont(INTRO_PARAGRAPH_FONT_FILE_NAME, paragraphLabel.fontName, paragraphLabel.fontSize);

	const auto labelNamesSize = labelNames.size();
	using lbSzType = decltype(labelNames.size());
	for (lbSzType i = 0; i < labelNamesSize; ++i) {
		_labels[i].reset(buildLabel(*labels[labelNames[i]], font));
	}

	const auto scaredButton = *_introForm->commandButtons[SCARED_BUTTON];
	_scared.reset(new GUI::ButtonWidget(this, scaredButton.left, scaredButton.top, scaredButton.width, scaredButton.height, scaredButton.caption, Common::U32String(""), kCmdContinue));

	const auto passwordLabel = *labels[PASSWORD_LABEL];
	auto& passwordFont = *buildWinFont(MAIN_MENU_PASSWORD_FONT_FILE_NAME, MAIN_MENU_PASSWORD_FONT_NAME, passwordLabel.fontSize);
	_password.reset(buildClickableLabel(passwordLabel, passwordFont, kCmdPassword));
}


void MainMenu::loadMainMenu() {
	const auto labels = _introForm->labels;
	const auto optionButtons = _introForm->optionButtons;

	const auto newGameLabel = *labels[NEW_GAME_LABEL];
	const auto quitLabel = *labels[QUIT_LABEL];

	const auto watchIntroOption = *optionButtons[WATCH_INTRO_OPTION];
	const auto skipIntroOption = *optionButtons[SKIP_INTRO_OPTION];

	auto& font = *buildTTFFont(MAIN_MENU_BUTTON_FONT_FILE_NAME, newGameLabel.fontSize);

	setBackground();

	_startGame.reset(buildClickableLabel(newGameLabel, font, kCmdNewGame));
	_quitGame.reset(buildClickableLabel(quitLabel, font, kCmdQuit));

	_options.reset(new GUI::RadiobuttonGroup(this));

	const int rbPosOffset = 20;
	_choices[0].reset(new GUI::RadiobuttonWidget(this, watchIntroOption.left - rbPosOffset, watchIntroOption.top, watchIntroOption.width + rbPosOffset, watchIntroOption.height, _options.get(), kWatchIntro, watchIntroOption.caption));
	_choices[1].reset(new GUI::RadiobuttonWidget(this, skipIntroOption.left - rbPosOffset, skipIntroOption.top, skipIntroOption.width + rbPosOffset, skipIntroOption.height, _options.get(), kSkipIntro, skipIntroOption.caption));
}

void MainMenu::unloadIntroScreen() {
	for (auto& label : _labels) {
		removeWidget(label.get());
		label.reset();
	}

	removeWidget(_scared.get());
	_scared.reset();

	g_gui.scheduleTopDialogRedraw();
}

void MainMenu::setBackground() {
	Common::File file;

	const auto img = *_introForm->images[INTRO_BACKGROUND_IMAGE];

	const auto picture = img.picture;
	const auto filename = picture.filename;
	if (!file.open(filename)) {
		error("sReadForm(): Error reading %s", filename.c_str());
	}

	file.skip(picture.location);
	uint32 file_size = file.readUint32LE();

	Common::SeekableSubReadStream mem(&file, file.pos(), file.pos() + file_size);

	Image::BitmapDecoder image;

	if (!image.loadStream(mem)) {
		error("sReadForm(): Error loading image %s", filename.c_str());
	}
	_background.reset(new GUI::GraphicsWidget(this, img.left, img.top, img.width, img.height));
	_background->setGfx(image.getSurface()->convertTo(g_system->getScreenFormat()));
}

void MainMenu::handleCommand(GUI::CommandSender *sender, uint32 command, uint32 data) {
	switch (command) {
	case kCmdContinue:
		unloadIntroScreen();
		loadMainMenu();
		break;
	case kCmdPassword:
		setResult(kCmdPassword);
		close();
		break;
	case kCmdQuit:
		setResult(kCmdQuit);
		close();
		break;
	case kCmdNewGame:
		setResult(_options->getValue());
		close();
		break;
	default:
		YahtzeeDialog::handleCommand(sender, command, data);
	}
}

void MainMenu::reflowLayout() {
	GUI::Dialog::reflowLayout();
}

void MainMenu::open() {
	YahtzeeDialog::open();
	loadIntroScreen();
}


GUI::GraphicsWidget* MainMenu::buildLabel(const Parser::Label& label, const Graphics::Font& font) {
	auto gw = new GUI::GraphicsWidget(this, label.left, label.top, label.width, label.height);
	Graphics::Surface surface;
	writeSurface(surface, label.width, label.height);
	writeLabel(surface, label, font);
	gw->setGfx(&surface);
	return gw;
}

ClickableImageWidget* MainMenu::buildClickableLabel(const Parser::Label& label, const Graphics::Font& font, uint32 cmd) {
	auto gw = new ClickableImageWidget(this, label.left, label.top, label.width, label.height, Common::U32String(""), cmd);
	Graphics::Surface surface;
	writeSurface(surface, label.width, label.height);
	writeLabel(surface, label, font);
	gw->setGfx(&surface);
	return gw;
}

GUI::GraphicsWidget* PasswordDialog::buildLabelWithBackgroundColor(const Parser::Label& label, const Graphics::Font& font, uint32 bkgColor) {
	auto gw = new GUI::GraphicsWidget(_container.get(), label.left, label.top, label.width, label.height);
	Graphics::Surface surface;
	writeSurface(surface, label.width, label.height);
	surface.fillRect(Common::Rect(0, 0, label.width, label.height), bkgColor);
	writeLabel(surface, label, font);
	gw->setGfx(&surface);
	return gw;
}



PasswordDialog::PasswordDialog(YahtzeeGame* vm) : Yahtzee::YahtzeeDialog(vm) {
	GUI::Dialog::_backgroundType = GUI::ThemeEngine::kDialogBackgroundSpecial;
	_passwordForm.reset(_vm->forms[PASSWORD_FORM]);
	_x = _passwordForm->clientLeft;
	_y = _passwordForm->clientTop;
	_w = _passwordForm->clientWidth;
	_h = _passwordForm->clientHeight;

	_container.reset(new ContainerImageWidget(this, 0, 0, _w, _h));

	//SurfacePtr surface(new Graphics::Surface());
	auto pixelFormat = g_system->getScreenFormat();
	const auto backColor = msColorToRGB(pixelFormat, _passwordForm->backColor);

	//surface->create(_w, _h, pixelFormat);

	//surface->fillRect(Common::Rect(0, 0, _w, _h), backColor);
	//_container->setGfx(surface.get());

	uint8 r, g, b;
	pixelFormat.colorToRGB(backColor, r, g, b);
	//_container->setBackgroundType(GUI::ThemeEngine::kWidgetBackgroundPlain);
	_container->setGfx(_w, _h, r, g, b);

	const auto enterPasswordLabel = *_passwordForm->labels[ENTER_PASSWORD_LABEL];
	auto& font = *buildWinFont(PASSWORD_FONT_FILE_NAME, PASSWORD_FONT_NAME, PASSWORD_FONT_SIZE);
	_passwordLabel.reset(buildLabelWithBackgroundColor(enterPasswordLabel, font, backColor));

	const auto passwordTextbox = *_passwordForm->textboxes[PASSWORD_TEXTBOX];
	_password.reset(new PasswordTextWidget(_container.get(), passwordTextbox.left, passwordTextbox.top, passwordTextbox.width, passwordTextbox.height, "", Common::U32String(""), 0, 0, '?'));

	const auto passwordOkButton = *_passwordForm->commandButtons[PASSWORD_OK_BUTTON];
	_ok.reset(new GUI::ButtonWidget(_container.get(), passwordOkButton.left, passwordOkButton.top, passwordOkButton.width, passwordOkButton.height, passwordOkButton.caption, Common::U32String(""), kCmdContinue));

	setFocusWidget(_password.get());
}

PasswordDialog::~PasswordDialog() {

}

void PasswordDialog::reflowLayout() {
	GUI::Dialog::reflowLayout();
}

void PasswordDialog::handleCommand(GUI::CommandSender* sender, uint32 cmd, uint32 data) {
	const auto password = _password->getEditString();

	if (password == PASSWORDS[0]) {
		setResult(YahtzeeChapters::START);
	} else if (password == PASSWORDS[1]) {
		setResult(YahtzeeChapters::CHOP_SUEY);
	} else if (password == PASSWORDS[2]) {
		setResult(YahtzeeChapters::LUBRICANT);
	} else if (password == PASSWORDS[3]) {
		setResult(YahtzeeChapters::STRESSED);
	} else if (password == PASSWORDS[4]) {
		setResult(YahtzeeChapters::INFIDEL);
	} else if (password == PASSWORDS[5]) {
		setResult(YahtzeeChapters::SNIFFER);
	} else if (password == PASSWORDS[6]) {
		setResult(YahtzeeChapters::UNTOWARD);
	} else if (password == PASSWORDS[7]) {
		setResult(YahtzeeChapters::CLIMAX);
	} else {
		setResult(YahtzeeChapters::START);
	}

	GUI::Dialog::handleCommand(sender, cmd, data);
}

DeathDialog::DeathDialog(YahtzeeGame* vm) : Yahtzee::YahtzeeDialog(vm) {
	GUI::Dialog::_backgroundType = GUI::ThemeEngine::kDialogBackgroundPlain;

	_deathForm.reset(_vm->forms[DEATH_FORM]);
	_x = _deathForm->clientLeft;
	_y = _deathForm->clientTop;
	_w = _deathForm->clientWidth;
	_h = _deathForm->clientHeight;


	const auto quitButton = *_deathForm->commandButtons[DEATH_QUIT_BUTTON];
	_quit.reset(new GUI::ButtonWidget(this, quitButton.left, quitButton.top, quitButton.width, quitButton.height, quitButton.caption, Common::U32String(""), kCmdQuit));

	setBackground();
}

DeathDialog::~DeathDialog() {};


void DeathDialog::handleCommand(GUI::CommandSender *sender, uint32 command, uint32 data) {
	switch (command) {
	case kCmdQuit:
		setResult(kCmdQuit);
		close();
		break;
	default:
		YahtzeeDialog::handleCommand(sender, command, data);
	}
}

void DeathDialog::reflowLayout() {
	GUI::Dialog::reflowLayout();
}

void DeathDialog::setBackground() {
	Common::File file;

	const auto img = *_deathForm->images[DEATH_IMAGE];
	const auto picture = img.picture;
	const auto filename = picture.filename;

	if (!file.open(filename)) {
		error("sReadForm(): Error reading %s", filename.c_str());
	}

	file.skip(picture.location);
	const uint32 fileSize = file.readUint32LE();

	Common::SeekableSubReadStream mem(&file, file.pos(), file.pos() + fileSize);

	Image::BitmapDecoder image;
	image.loadStream(mem);

	_hat.reset(new GUI::GraphicsWidget(this, img.left, img.top, img.width, img.height));
	_hat->setGfx(image.getSurface()->convertTo(g_system->getScreenFormat()));
}





}
