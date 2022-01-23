/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "yahtzee/yahtzee.h"
#include "yahtzee/console.h"

#include "audio/decoders/wave.h"
#include "audio/audiostream.h"

#include "common/algorithm.h"
#include "common/archive.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/keyboard.h"
#include "common/ptr.h"
#include "common/substream.h"
#include "common/system.h"
#include "common/timer.h"
#include "common/rect.h"

#include "engines/util.h"

#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fonts/ttf.h"
#include "graphics/fonts/winfont.h"
#include "graphics/fontman.h"
#include "graphics/palette.h"
#include "graphics/surface.h"
#include "gui/message.h"

#include "image/bmp.h"

#include "yahtzee/gui/dialogs.h"
#include "yahtzee/controls.h"
#include "yahtzee/util.h"


namespace Yahtzee {


Common::Rect buildRect(const Parser::Form& form) {
	return Common::Rect(form.left, form.top, form.left + form.width, form.top + form.height);
}

Common::Rect buildRect(const Parser::Form& form, uint32 x1_offset, uint32 y1_offset, uint32 x2_offset, uint32 y2_offset) {
	return Common::Rect(form.left + x1_offset, form.top + y1_offset, form.left + x2_offset, form.top + y2_offset);
}

template<class T, class U>
Common::Rect buildRect(const T& form, const U& vc) {
	return Common::Rect(form.clientLeft + vc.left, form.clientTop + vc.top, form.clientLeft + vc.left + vc.width, form.clientTop + vc.top + vc.height);
}

template<class T>
Common::Rect buildRect(const Parser::Form& form, const Common::String& imgName) {
	return Common::Rect();
}


template<>
Common::Rect buildRect<Parser::Image>(const Parser::Form& form, const Common::String& imgName) {
	Parser::Image img = *form.images[imgName];
	return buildRect(form, img);
}

template<>
Common::Rect buildRect<Parser::Label>(const Parser::Form& form, const Common::String& lblName) {
	return buildRect(form, *form.labels[lblName]);
}

template<class T>
using HashMapT = Common::HashMap<Common::String, T>;

template<class T>
bool isSelected(const Common::String& objName, const Parser::Form& form, const Common::Point& cur) {
	auto itemRect = buildRect<T>(form, objName);
	return itemRect.contains(cur);
};



YahtzeeGame::YahtzeeGame(OSystem *syst, const ADGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc), _console(nullptr), _image(nullptr), forms(),
		fonts(), _count(0), _interval(1), mainMenuForm(nullptr), _currentScene(0), introForm(nullptr),
		currentForm(nullptr), passwordForm(nullptr), inventoryForm(nullptr), _md(nullptr), inventoryActive(false), sceneActive(true),
	_timerInstalled(false), itemForeColor(0), commFont(nullptr), commForeColor(0), itemFont(nullptr),
	selectedObject(nullptr), useObject(nullptr), currentAction(yLook) {

	DebugMan.addDebugChannel(kDebugGeneral, "general", "General debug level");


	//selectedObject = new YahtzeeObject("", Common::Rect(0, 0, 0, 0));

	// cutscene - 2, 3, 4
	_introScenes = {
			"Form2",
			"Form3",
			"Form4"
	};

	_chapterOne = {
			"Form7",
			"Form8",
			"Form10",
			"Form11",
			"Form12"
	};


	// 4 rooms
	// cutscene - 12

	_chapterTwo = {
			"Form13",
			"Form14",
			"Form15",
			"Form16"
	};

	// 6 rooms
	// cutscene - 25 26 27
	_chapterThree = {
			"Form18",
			"Form19",
			"Form20",
			"Form21",
			"Form22",
			"Form23",
			"Form24",
			"Form25",
			"Form26",
			"Form27"
	};

	// 6 rooms
	// cutscene 35
	_chapterFour = {
			"Form28",
			"Form29",
			"Form30",
			"Form31",
			"Form32",
			"Form33",
			"Form34",
			"Form35"
	};

	// interactive
	_chapterFive = {
			"Form36",
	};

	// room - 37
	// cutscene- 38
	// 10 rooms
	_chapterSix = {
			"Form37",
			"Form38",
			"Form39",
			"Form40",
			"Form41",
			"Form42",
			"Form43",
			"Form44",
			"Form45",
			"Form46",
			"Form47",
			"Form48",
			"Form49"
	};

	// room 50 51
	// cutscene 52 53 54
	_chapterSeven = {
			"Form50",
			"Form51",
			"Form52",
			"Form53",
			"Form54"
	};

	// room 55
	// cutscene 56 57 58 59 60
	_chapterEight = {
			"Form55",
			"Form56",
			"Form57",
			"Form58",
			"Form59",
			"Form60"
	};

	// Stage
	// Segment
	// Sequence Area
}

YahtzeeGame::~YahtzeeGame() {
	//_console is deleted by Engine
}


Common::Error YahtzeeGame::run() {
	// 24 bit graphics
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT, &PIXEL_FORMAT_24_BIT);
	initScreen();
	//_image = Image::BitmapDecoder();
	_console.reset(new Console());
	setDebugger(_console.release());

	CursorMan.replaceCursor(MOUSECURSOR_SCI, 11, 16, 0, 0, 0);
	CursorMan.replaceCursorPalette(cursorPalette, 0, 3);
	CursorMan.showMouse(true);

	//loadFonts();
	forms = Parser::readForms();

	inventoryForm = forms[INVENTORY_FORM];
	initInventory();

	auto commLabel = *inventoryForm->labels[COMM_CAPTION_LABEL];
	commFont = buildWinFont(COMM_LABEL_FONT_FILE_NAME, commLabel.fontName, commLabel.fontSize);
	commForeColor = msColorToRGB(g_system->getScreenFormat(), commLabel.foreColor);
	commCaption = commLabel.caption;

	auto itemLabel = *inventoryForm->labels[ITEM_CAPTION_LABEL];
	itemFont = buildWinFont(ITEM_LABEL_FONT_FILE_NAME, itemLabel.fontName, itemLabel.fontSize);
	itemForeColor = msColorToRGB(g_system->getScreenFormat(), itemLabel.foreColor);
	itemCaption = itemLabel.caption;


	bottomMenuQuit = buildRect<Parser::Image>(*inventoryForm, BOTTOM_MENU_QUIT);
	bottomMenuLook = buildRect<Parser::Image>(*inventoryForm, BOTTOM_MENU_LOOK);
	bottomMenuOperate = buildRect<Parser::Image>(*inventoryForm, BOTTOM_MENU_OPERATE);
	bottomMenuSpeakTo = buildRect<Parser::Image>(*inventoryForm, BOTTOM_MENU_SPEAK);
	bottomMenuUse = buildRect<Parser::Image>(*inventoryForm, BOTTOM_MENU_USE);

	//sceneArea = buildRect(*currentForm);

	inventoryArea = buildRect(*inventoryForm, INVENTORY_X1, INVENTORY_Y1, INVENTORY_X2, INVENTORY_Y2);
	actionArea = buildRect(*inventoryForm, ACTION_X1, ACTION_Y1, ACTION_X2, ACTION_Y2);

	_actions.clear();
	_actions.push(ShowScene);

	Yahtzee::YahtzeeDialog* md, *pd, *dd;
	md = new Yahtzee::MainMenu(this);
	int frame = 0;

	int test = runDialog(*md);
	switch (test) {
	case kCmdPassword:
		pd = new Yahtzee::PasswordDialog(this);
		frame = runDialog(*pd);
		break;
	case kWatchIntro:
		frame = 0;
		playIntro();
		break;
	case kSkipIntro:
		frame = 1;
		firstScene();
		break;
	case kCmdQuit:
		quitGame();
		break;
	default:
		break;
	}




	Common::Event event;
	auto contains = [&](const Common::Rect& area) {
		return area.contains(event.mouse);
	};

	Common::Rect itemRect;


	Common::String caption;
	while (!shouldQuit()) {
		while (_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
				warning("QUIT");
				quitGame();
				break;
			case Common::EVENT_KEYDOWN:
				processKeyPress(event.kbd.keycode);
				break;
			case Common::EVENT_MOUSEMOVE:
				if (selectedObject && contains(selectedObject->area))
					break;
				if (contains(sceneArea)) {
					selectedObject = nullptr;
					for (const auto& obj : sceneObjects) {
						if (obj._value->area.contains(event.mouse)) {
							selectedObject = obj._value;
						}
					}

					if (selectedObject) {
						itemCaption = selectedObject->caption;
						updateItem();
					} else {
						clearItem();
					}
				} else if (contains(inventoryArea)) {
					selectedObject = nullptr;
					for (const auto& obj : inventoryObjects) {
						if (obj._value->area.contains(event.mouse)) {
							selectedObject = obj._value;
						}
					}

					if (selectedObject) {
						itemCaption = selectedObject->caption;
						updateItem();
					} else {
						clearItem();
					}
				}
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (contains(sceneArea)) {
					for (const auto& obj : sceneObjects) {
						if (obj._value->area.contains(event.mouse)) {
							selectedObject = obj._value;
						}
					}

					if (!selectedObject)
						break;

					warning(selectedObject->id.c_str());

					switch(currentAction) {
					case yLook:
						testOps = selectedObject->lookOps;
						break;
					case yOperate:
						testOps = selectedObject->operateOps;
						break;
					case ySpeak:
						testOps = selectedObject->speakOps;
						break;
					case yUse:
						testOps = selectedObject->useOps[useObject->caption];
						break;
					default:
						error("An action should be selected");
						break;
					}


					for (auto& op : testOps) {
						actOnOp(*op);
					}

				} else if (contains(actionArea)) {
					if (contains(bottomMenuLook)) {
						currentAction = yLook;
					} else if (contains(bottomMenuOperate)) {
						currentAction = yOperate;
					} else if (contains(bottomMenuSpeakTo)) {
						currentAction = ySpeak;
					} else if (contains(bottomMenuUse)) {
						currentAction = yUse;
					} else {
						break;
					}

					commCaption = selectCommCaption(currentAction);

					updateComm();
				} else if (contains(inventoryArea)) {
					warning("INVENTORY AREA CLICKED");

					for (const auto& obj : inventoryObjects) {
						if (obj._value->area.contains(event.mouse)) {
							selectedObject = inventoryObjects[obj._key];
						}
					}

					if (!selectedObject)
						break;

					switch(currentAction) {
					case yLook:
						testOps = selectedObject->lookOps;
						break;
					case yOperate:
						testOps = selectedObject->operateOps;
						break;
					case ySpeak:
						testOps = selectedObject->speakOps;
						break;
					case yUse:
						useObject = selectedObject;
						useItemCaption = useObject->caption;
						updateUseItem();
						goto endInventory;
					default:
						error("An action should be selected");
						break;
					}


					for (auto& op : testOps) {
						actOnOp(*op);
					}

					endInventory:
					;
				} else if (contains(bottomMenuQuit)) {
					quitGame();
				} else {
					_actions.push(PlaySound);
				}
				break;
			default:
				break;
			}
		}

		while (!_actions.empty()) {
			switch (_actions.pop()) {
			case Redraw:
				updateForm(currentForm);
				updateForm(inventoryForm);
				break;
/*			case ShowScene:
				showScene();
				break;
			case UpdateScene:
				updateScene();
				break;
			case ChangeScene:
				changeScene();
				break;
*/			case PlaySound:
				playSound();
				break;
			case DisplayMsg:
				if (_timerInstalled) {
					_system->getTimerManager()->removeTimerProc(onTimer);
					_timerInstalled = false;
					displayMessage(msgQueue.pop());
					_system->getTimerManager()->installTimerProc(onTimer, _interval * 1000, this, "timer");
					_timerInstalled = true;
				} else {
					displayMessage(msgQueue.pop());
				}
				break;
			default:
				break;
			}
		}
		_system->updateScreen();
		_system->delayMillis(REFRESH_DELAY_TIME);
	}

	_system->getTimerManager()->removeTimerProc(onTimer);
	stopSound();

	return Common::kNoError;
}

void YahtzeeGame::actOnOp(Op& op) {
	switch(op.opCode) {
	case Hide:
		currentForm->images[static_cast<HideOp*>(&op)->objName]->visible = false;
		//_actions.push(Redraw);
		updateForm(currentForm);
		updateForm(inventoryForm);
		break;
	case Show:
		inventoryForm->images[static_cast<ShowOp*>(&op)->objName]->visible = true;
		//_actions.push(Redraw);
		updateForm(currentForm);
		updateForm(inventoryForm);
		break;
	case Dialog:
		msgQueue.push(static_cast<DialogOp*>(&op)->dialog);
		_actions.push(DisplayMsg);
		break;
	}
}

Common::String YahtzeeGame::selectCommCaption(YahtzeeAction act) {
	Common::String caption;
	switch (act) {
	case yLook:
		caption = LOOK_AT_CAPTION;
		break;
	case yOperate:
		caption = OPERATE_CAPTION;
		break;
	case ySpeak:
		caption = SPEAK_CAPTION;
		break;
	case yUse:
		caption = USE_CAPTION;
		break;
	default:
		error("currentAction should not be empty");
		break;
	}
	return caption;
}

void YahtzeeGame::displayMessage(const Common::String& msg) {
	_md = new GUI::MessageDialog(msg);
	playSound();
	_md->runModal();
}

//void YahtzeeGame::processMouseDown()

void YahtzeeGame::processKeyPress(Common::KeyCode key) {
	Yahtzee::YahtzeeDialog* pd, *dd;

	switch (key) {
	case Common::KEYCODE_LEFT:
		previousScene();
		break;
	case Common::KEYCODE_RIGHT:
		nextScene();
		break;
	case Common::KEYCODE_d:
		dd = new Yahtzee::DeathDialog(this);
		runDialog(*dd);
		quitGame();
		break;
	case Common::KEYCODE_i:
		if (inventoryActive)
			hideInventory();
		else
			showInventory();
		break;
	case Common::KEYCODE_p:
		pd = new Yahtzee::PasswordDialog(this);
		runDialog(*pd);
		break;
	case Common::KEYCODE_q:
		quitGame();
		break;
	case Common::KEYCODE_s:
		if (sceneActive)
			hideScene();
		else
			showScene();
		break;
	default:
		break;
	}
}

//TODO: Fix this hack!
const Common::String INTRO_TIMER = "Timer1";


void YahtzeeGame::suspendTimer() {
	_system->getTimerManager()->removeTimerProc(onTimer);
	_timerInstalled = false;
}

void YahtzeeGame::restoreTimer() {
	_system->getTimerManager()->installTimerProc(onTimer,  _interval * 1000, this, INTRO_TIMER);
	_timerInstalled = true;
}

void YahtzeeGame::initInventory() {
	auto screen = _system->lockScreen();

	const auto BACKGROUND_COLOR = 0xFFFF;

	screen->fillRect(Common::Rect(inventoryForm->clientLeft, inventoryForm->clientTop, inventoryForm->clientLeft + inventoryForm->clientWidth, inventoryForm->clientTop + inventoryForm->clientHeight), BACKGROUND_COLOR);

	_system->unlockScreen();
	_system->updateScreen();
}

void YahtzeeGame::initScreen() {
	auto screen = _system->lockScreen();

	const auto BACKGROUND_COLOR = 0xFFFF;
	screen->fillRect(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), BACKGROUND_COLOR);

	_system->unlockScreen();
	_system->updateScreen();
}

void YahtzeeGame::playIntro() {
	const Common::String INTRO_FORM1 = "Form2";
	const Common::String INTRO_TIMER = "Timer1";

	const Common::String YAHTZEE_FACE1 = "Image1";
	const Common::String YAHTZEE_FACE2 = "Image2";

	Common::Array<TimedOp> arr = {
		TimedOp {5, OpList {new DialogOp("DING-DONG", 0, "Door")}},
		TimedOp {6, OpList {new HideOp(YAHTZEE_FACE1), new ShowOp(YAHTZEE_FACE2)}},
		TimedOp {8, OpList {new DialogOp("Bugger. The door.", 0, "Arthur")}},
		TimedOp {9, OpList {new DialogOp("PLACEHOLDER TEXT; TO BE REMOVED", 0, "Programmer")}}
	};

	for (auto& a : arr) {
		currentTimer.ops.push(a);
	}

	currentForm = forms[INTRO_FORM1];

	//initScreen();

	updateForm(currentForm);


	_interval = currentForm->timers[INTRO_TIMER]->interval;
	_system->getTimerManager()->installTimerProc(onTimer, _interval * 1000, this, "timer");
}

/*
YahtzeeObject readObject(Common::SeekableReadStream &stream) {
	YahtzeeObject yo;
	yo.id = stream.read3
	readVisible<Parser::Form>(form, stream);
	readPosition<Parser::Form>(form, stream);
	readColor<Parser::Form>(form, stream);
	readCaption<Parser::Form>(form, stream);
	form->borderStyle = stream.readUint32BE();
	form->clientHeight = stream.readUint32BE();
	form->clientLeft = stream.readUint32BE();
	form->clientWidth = stream.readUint32BE();
	form->clientTop = stream.readUint32BE();
	form->linkTopic = stream.readPascalString();
	form->scaleHeight = stream.readUint32BE();
	form->scaleWidth = stream.readUint32BE();

	form->labels = readControls<Parser::Label>(stream);
	form->timers = readControls<Parser::Timer>(stream);
	form->commandButtons = readControls<Parser::CommandButton>(stream);
	form->optionButtons = readControls<Parser::OptionButton>(stream);
	form->textboxes = readControls<Parser::TextBox>(stream);
	form->images = readControls<Parser::Image>(stream);

	bool hasPicture = stream.readByte();
	if (hasPicture) {
		form->picture = readPicture(stream);
	}
	return form;
}
*/

void YahtzeeGame::firstScene() {
	_currentScene = 0;
	currentForm = forms[_chapterOne[_currentScene]];

	//initScreen();

	updateForm(currentForm);

	const Common::String IMAGE1 = "Image1";

	OpMap opMap;
	opMap["MUG"] = OpList {new DialogOp("I'm not thirsty. Well, actually I am, but not for that stuff.", 0, "Arthur")};

	sceneObjects[IMAGE1] = new YahtzeeObject(IMAGE1, "MUG", buildRect<Parser::Image>(*currentForm, IMAGE1), {
			new DialogOp("It's a little mug with brown stuff inside.", 0, "Arthur")
	}, {
			new HideOp(IMAGE1),
			new ShowOp("Image5"),
			new DialogOp("Okay. Why not.", 0, "Arthur"),
	}, {
			new DialogOp("People might talk.", 0, "Arthur"),
	}, opMap);


	const Common::String IMAGE2 = "Image2";
	opMap["MUG"] = OpList {new DialogOp("I'm not thirsty. Well, actually I am, but not for that stuff.", 0, "Arthur")};

	sceneObjects[IMAGE2] = new YahtzeeObject(IMAGE2, "ARTHUR", buildRect<Parser::Image>(*currentForm, IMAGE2), {
			new DialogOp("It's me! Arthur Yahtzee! Hero-type guy!", 0, "Arthur")
	}, {
			new DialogOp("You filthy, evil minded pervert.", 0, "Arthur"),
	}, {
			new DialogOp("People might talk.", 0, "Arthur"),
	}, opMap);


	const Common::String LABEL1 = "Label1";
	opMap["MUG"] = OpList {new DialogOp("He does look rather thirsty, but I don't think he'd appreciate me.", 0, "Arthur")};

	sceneObjects[LABEL1] = new YahtzeeObject(LABEL1, "ILL LOOKING BLOKE", buildRect<Parser::Label>(*currentForm, LABEL1), {
			new DialogOp("A skeleton. In my cell. Chained up. Have these mutants no originality?", 0, "Arthur")
	}, {
			new DialogOp("You filthy, evil minded pervert.", 0, "Arthur"),
	}, {
			new DialogOp("So, what are you in for?", 0, "Arthur"),
	}, opMap);


	const Common::String LABEL2 = "Label2";
	opMap["MUG"] = OpList {
		new DialogOp("Yah boo! All mutants smell of fifteenth century London!", 0, "Arthur"),
		new DialogOp("And they probably perform unspecific obscene acts with small dalmation puppies!", 0, "Arthur"),
		new DialogOp("And...", 0, "Arthur")
	};

	sceneObjects[LABEL2] = new YahtzeeObject(LABEL2, "BARS", buildRect<Parser::Label>(*currentForm, LABEL2), {
			new DialogOp("I can see a guard through there. Isn't this fun?", 0, "Arthur")
	}, {
			new DialogOp("Yes, I think I'll just evolve the strength of ten men and tear the bars from their sockets. Yeah.", 0, "Arthur"),
	}, {
			new DialogOp("People might talk.", 0, "Arthur"),
	}, opMap);


	const Common::String INVENTORY_IMAGE5  = "Image5";
	opMap["MUG"] = OpList {new DialogOp("TODO: Remove this, replace by different object", 0, "Arthur")};

	inventoryObjects[INVENTORY_IMAGE5] = new YahtzeeObject(INVENTORY_IMAGE5, "MUG", buildRect<Parser::Image>(*inventoryForm, INVENTORY_IMAGE5), {
			new DialogOp("Looks like a little mug to me.", 0, "Arthur")
	}, {
			new DialogOp("The brown water inside doesn't look too appetising.", 0, "Arthur"),
	}, {
			new DialogOp("People might talk.", 0, "Arthur"),
	}, opMap);


	/*
	 *  If  Label3.Caption = 1 Then Image2.Visible = False: Image3.Visible = True
		If  Label3.Caption = 2 Then Image3.Visible = False: Image4.Visible = True
		If  Label3.Caption = 3 Then Image4.Visible = False: Image3.Visible = True
		If  Label3.Caption = 4 Then Image3.Visible = False: Image4.Visible = True
		If  Label3.Caption = 5 Then Image4.Visible = False: Image3.Visible = True
		If  Label3.Caption = 6 Then Image3.Visible = False: Image4.Visible = True
		If  Label3.Caption = 7 Then Image4.Visible = False: Image3.Visible = True
		If  Label3.Caption = 8 Then Image3.Visible = False: Image4.Visible = True
	 */

	const Common::String IMAGE3 = "Image3";
	const Common::String IMAGE4 = "Image4";

	Common::Array<TimedOp> arr = {
		TimedOp {1, OpList {new HideOp(IMAGE2), new ShowOp(IMAGE3)}},
		TimedOp {2, OpList {new HideOp(IMAGE3), new ShowOp(IMAGE4)}},
		TimedOp {3, OpList {new HideOp(IMAGE4), new ShowOp(IMAGE3)}},
		TimedOp {4, OpList {new HideOp(IMAGE3), new ShowOp(IMAGE4)}},
		TimedOp {5, OpList {new HideOp(IMAGE4), new ShowOp(IMAGE3)}},
		TimedOp {6, OpList {new HideOp(IMAGE3), new ShowOp(IMAGE4)}},
		TimedOp {7, OpList {new HideOp(IMAGE4), new ShowOp(IMAGE3)}},
		TimedOp {8, OpList {new HideOp(IMAGE3), new ShowOp(IMAGE4)}},
		TimedOp {9, OpList {
			new DialogOp("Yah boo! All mutants smell of fifteenth century London!", 0, "Arthur"),
			new DialogOp("And they probably perform unspecific obscene acts with small dalmation puppies!", 0, "Arthur"),
			new DialogOp("And...", 0, "Arthur")}},
	};

	for (auto& a : arr) {
		currentTimer.ops.push(a);
	}
/*
	for (const auto& image : currentForm->images) {
		if (image._value->visible)
			sceneObjects[image._key] = new YahtzeeObject(image._key, buildRect<Parser::Image>(*currentForm, image._key), lookOps, operateOps, speakOps, useOps);
	}

	for (const auto& label : currentForm->labels) {
		sceneObjects[label._key] = new YahtzeeObject(label._key, buildRect<Parser::Label>(*currentForm, label._key), lookOps, operateOps, speakOps, useOps);
	}
*/
	sceneArea = buildRect(*currentForm);
	showInventory();
}

void YahtzeeGame::previousScene() {
	--_currentScene;
	hideScene();
	currentForm = forms[_chapterOne[_currentScene]];
	updateForm(currentForm);
}

void YahtzeeGame::nextScene() {
	++_currentScene;
	hideScene();
	currentForm = forms[_chapterOne[_currentScene]];
	updateForm(currentForm);
}

void YahtzeeGame::showScene() {
	sceneActive = true;
	updateForm(currentForm);
}

void YahtzeeGame::hideScene() {
	auto screen = _system->lockScreen();
	sceneActive = false;

	const auto BACKGROUND_COLOR = 0x0000;
	screen->fillRect(buildRect(*currentForm), BACKGROUND_COLOR);

	_system->unlockScreen();
	_system->updateScreen();
}

void YahtzeeGame::updateComm() {
	auto screen = _system->lockScreen();

	auto commLabel = *inventoryForm->labels[COMM_CAPTION_LABEL];

	Graphics::Surface surface;
	surface.create(commLabel.width, commLabel.height, g_system->getScreenFormat());
	commFont->drawString(&surface, commCaption, 0, 0, commLabel.width, commForeColor, Graphics::kTextAlignLeft);
	screen->copyRectToSurface(surface, inventoryForm->clientLeft + commLabel.left, inventoryForm->clientTop + commLabel.top, Common::Rect(0, 0, commLabel.width, commLabel.height));

	_system->unlockScreen();
	_system->updateScreen();
}

void YahtzeeGame::clearComm() {
	auto screen = _system->lockScreen();
	auto commLabel = *inventoryForm->labels[COMM_CAPTION_LABEL];

	const auto BACKGROUND_COLOR = 0x0000;
	screen->fillRect(Common::Rect(inventoryForm->clientLeft + commLabel.left, inventoryForm->clientTop + commLabel.top, inventoryForm->clientLeft + commLabel.left + commLabel.width, inventoryForm->clientTop + commLabel.top + commLabel.height), BACKGROUND_COLOR);

	_system->unlockScreen();
	_system->updateScreen();
}

void YahtzeeGame::updateItem() {
	auto screen = _system->lockScreen();
	auto itemLabel = *inventoryForm->labels[ITEM_CAPTION_LABEL];

	Graphics::Surface surface;
	surface.create(itemLabel.width, itemLabel.height, g_system->getScreenFormat());
	itemFont->drawString(&surface, itemCaption, 0, 0, itemLabel.width, itemForeColor, Graphics::kTextAlignLeft);
	screen->copyRectToSurface(surface, inventoryForm->clientLeft + itemLabel.left, inventoryForm->clientTop + itemLabel.top, Common::Rect(0, 0, itemLabel.width, itemLabel.height));


	_system->unlockScreen();
	_system->updateScreen();
}


void YahtzeeGame::clearItem() {
	auto screen = _system->lockScreen();
	auto itemLabel = *inventoryForm->labels[ITEM_CAPTION_LABEL];
	const auto BACKGROUND_COLOR = 0x0000;

	screen->fillRect(buildRect(*inventoryForm, itemLabel), BACKGROUND_COLOR);

	_system->unlockScreen();
	_system->updateScreen();
}


void YahtzeeGame::updateUseItem() {
	auto screen = _system->lockScreen();
	auto itemLabel = *inventoryForm->labels[USE_ITEM_CAPTION_LABEL];

	Graphics::Surface surface;
	surface.create(itemLabel.width, itemLabel.height, g_system->getScreenFormat());
	itemFont->drawString(&surface, useItemCaption, 0, 0, itemLabel.width, itemForeColor, Graphics::kTextAlignLeft);
	screen->copyRectToSurface(surface, inventoryForm->left + itemLabel.left, inventoryForm->top + itemLabel.top, Common::Rect(0, 0, itemLabel.width, itemLabel.height));


	_system->unlockScreen();
	_system->updateScreen();
}

void YahtzeeGame::clearUseItem() {
	auto screen = _system->lockScreen();
	auto itemLabel = *inventoryForm->labels[USE_ITEM_CAPTION_LABEL];
	const auto BACKGROUND_COLOR = 0x0000;

	screen->fillRect(buildRect(*inventoryForm, itemLabel), BACKGROUND_COLOR);

	_system->unlockScreen();
	_system->updateScreen();
}


void YahtzeeGame::showInventory() {
	inventoryActive = true;

	updateForm(inventoryForm);
	updateComm();
}

void YahtzeeGame::hideInventory() {
	auto screen = _system->lockScreen();

	inventoryActive = false;
	auto const BACKGROUND_COLOR = 0x0000;
	screen->fillRect(buildRect(*inventoryForm), BACKGROUND_COLOR);

	_system->unlockScreen();
	_system->updateScreen();
}

void copyImageToSurface(Graphics::Surface& surface, const Image::BitmapDecoder& img, int32 left, int32 top, int32 width, int32 height) {
	Graphics::Surface imageSfc = *img.getSurface();
	auto systemPixelFormat = surface.format;
	if (imageSfc.format != systemPixelFormat) {
		imageSfc.convertToInPlace(systemPixelFormat);
	}
	surface.copyRectToSurface(imageSfc, left, top, Common::Rect(0, 0, width, height));
}

//void copy(const Graphics::Surface& surface, const )

void YahtzeeGame::updateForm(Parser::Form* form) {
	auto screen = _system->lockScreen();
	auto copyImageToScreen = [&](uint32 left, uint32 top, uint32 width, uint32 height) {
		copyImageToSurface(*screen, *_image, left, top, width, height);
	};

	const auto BACKGROUND_COLOR = 0xFFFF;

	screen->fillRect(Common::Rect(form->clientLeft, form->clientTop, form->clientLeft + form->clientWidth, form->clientTop + form->clientHeight), BACKGROUND_COLOR);

	_image = loadPicture(form->picture);
	copyImageToScreen(form->clientLeft, form->clientTop, form->clientWidth, form->clientHeight);

	Parser::Image img;

	for (auto it : form->images) {
		img = *it._value;
		if (!img.visible)
			continue;

		_image = loadPicture(img.picture);

		copyImageToScreen(form->clientLeft + img.left, form->clientTop + img.top, img.width, img.height);
	}
	_system->unlockScreen();
	_system->updateScreen();
}

void YahtzeeGame::launchPasswordScreen() {
	auto pm = new Yahtzee::PasswordDialog(this);
	pm->runModal();
}

Graphics::Font* YahtzeeGame::loadWinFont(const Common::String& fontFileName, const Common::String& fontName, double fontSize) {
	auto winfont = new Graphics::WinFont();
	if (!winfont->loadFromFON(fontFileName, Graphics::WinFontDirEntry(fontName, fontSize))) {
		error("Couldn't load %s", fontFileName.c_str());
	}
	return winfont;
}
/*
void YahtzeeGame::loadFonts() {
	fonts[MS_SERIF_NAME] = loadWinFont(MS_SERIF_BOLD, MS_SERIF_NAME, MS_SERIF_FONT_SIZE);
	fonts[MS_SANS_SERIF_NAME] = loadWinFont(MS_SANS_SERIF_BOLD, MS_SANS_SERIF_NAME, MS_SANS_SERIF_FONT_SIZE);
	fonts[DEFAULT_FONT_NAME] = loadWinFont(MS_SANS_SERIF_BOLD, MS_SANS_SERIF_NAME, DEFAULT_FONT_SIZE);


	font = new Graphics::WinFont();
	if (!font->loadFromFON(MS_SERIF_BOLD, Graphics::WinFontDirEntry(MS_SANS_SERIF_NAME, DEFAULT_FONT_SIZE))) {
		error("Couldn't load %s", MS_SANS_SERIF.c_str());
	}
	fonts[MS_SANS_SERIF_NAME] = font;

	Common::File file;
	if (!file.open(MS_TIMES_NEW_ROMAN_BOLD_ITALIC)) {
		error("sReadForm(): Error reading %s", MS_TIMES_NEW_ROMAN_BOLD_ITALIC.c_str());
	}

	fonts[MS_TIMES_NEW_ROMAN_NAME] = Graphics::loadTTFFont(file, MS_TIMES_NEW_ROMAN_BOLD_FONT_SIZE * 20 / 15);
}
*/

void YahtzeeGame::loadImage(const Common::String &dirname, const Common::String &filename) {
	auto name = dirname + "/" + filename;
	debugC(1, kDebugGeneral, "%s : %s", __FUNCTION__, name.c_str());
	Common::File file;
	if (!file.open(name))
		error("unable to load image %s", name.c_str());

	_image->loadStream(file);
}

void YahtzeeGame::drawScreen() {
	// Check whether this has to be somewhere else
	auto form1 = forms["FORM1"];
	const int FORM_WIDTH = form1->scaleWidth;
	const int FORM_HEIGHT = form1->scaleHeight;

	/*
	debugC(1, kDebugGeneral, "%s : %s", __FUNCTION__, _image ? "YES" : "NO");
	if (!_image)
		error("sDrawScreen(): image not loaded");
	 */

	auto screen = _system->lockScreen();
	screen->fillRect(Common::Rect(0, 0, _system->getWidth(), _system->getHeight()), 0);

	auto surface = _image->getSurface();

	int w = CLIP<int>(surface->w, 0, FORM_WIDTH);
	int h = CLIP<int>(surface->h, 0, FORM_HEIGHT);

	int x = (FORM_WIDTH - w) / 2;
	int y = (FORM_HEIGHT - h) / 2;

	screen->copyRectToSurface(*surface, x, y, Common::Rect(0, 0, w, h));

	_system->unlockScreen();
	_system->getPaletteManager()->setPalette(_image->getPalette(), 0, MAX_COLORS);
	_system->updateScreen();

}

void YahtzeeGame::playSound() {
	const auto name = DING_SOUND_FILENAME;
	debugC(3, kDebugGeneral, "%s : %s", __FUNCTION__, name.c_str());
	Common::ScopedPtr<Common::File> file(new Common::File());
	if (!file->open(name)) {
		error("unable to load sound %s", name.c_str());
	}

	Common::ScopedPtr<Audio::AudioStream> audioStream(Audio::makeWAVStream(file.release(), DisposeAfterUse::YES));
	stopSound();
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, audioStream.release(), -1, Audio::Mixer::kMaxChannelVolume);
}

void YahtzeeGame::stopSound() {
	debugC(3, kDebugGeneral, "%s", __FUNCTION__);
	if (_mixer->isSoundHandleActive(_soundHandle))
		_mixer->stopHandle(_soundHandle);
}




void YahtzeeGame::processTimer() {
	TimedOp tOp = currentTimer.ops.front();
	if (_count++ == tOp.time) {
		for (auto& op : tOp.ops) {
			actOnOp(*op);
		}
		currentTimer.ops.pop();
	}
	/*
	const Common::String YAHTZEE_FACE = "Image1";
	const Common::String YAHTZEE_FLIPPED_FACE = "Image2";
	const Common::String DIALOG1 = "DING-DONG";
	const Common::String DIALOG2 = "Bugger. The door.";

	switch(_count++) {
	case 5:
		msgQueue.push(DIALOG1);
		_actions.push(DisplayMsg);
		break;
	case 6:
		currentForm->images[YAHTZEE_FLIPPED_FACE]->visible = true;
		currentForm->images[YAHTZEE_FACE]->visible = false;
		_actions.push(Redraw);
		break;
	case 8:
		msgQueue.push(DIALOG2);
		_actions.push(DisplayMsg);
		break;
	case 9:
		g_system->getTimerManager()->removeTimerProc(onTimer);
		_timerInstalled = false;

		_count = 0;
		quitGame();
		break;
	}
	*/
}

void YahtzeeGame::onTimer(void *arg) {
	((YahtzeeGame*)arg)->processTimer();
}


/*
YahtzeeObject* YahtzeeGame::getClickedObject(const Parser::Form& form, const Common::Point& cursor) {
	for (const auto& objName : sceneImages) {
		auto itemRect = buildRect<Parser::Image>(form, objName);
		if (itemRect.contains(cursor))
			return new YahtzeeObject(objName, itemRect);
	}

	for (const auto& objName : sceneLabels) {
		auto itemRect = buildRect<Parser::Label>(form, objName);
		if (itemRect.contains(cursor))
			return new YahtzeeObject(objName, itemRect);
	}

	return new YahtzeeObject("", Common::Rect(0, 0, 0, 0));
}


YahtzeeObject* YahtzeeGame::getClickedInventoryObject(const Parser::Form& form, const Common::Point& cursor) {
	for (const auto& objName : items) {
		auto itemRect = buildRect<Parser::Image>(form, objName);
		if (itemRect.contains(cursor)) {
			return new YahtzeeObject(objName, itemRect);
		}
	}

	return new YahtzeeObject("", Common::Rect(0, 0, 0, 0));
}
*/

/*
Common::String YahtzeeGame::getClickedObjectName(const Parser::Form& form, Common::Point cursor) {
	for (const auto& objName : sceneImages)
		if (isSelected<Parser::Image>(objName, form, cursor))
			return objName;

	for (const auto& objName : sceneLabels)
		if (isSelected<Parser::Label>(objName, form, cursor))
			return objName;

	return "";
}


Common::String YahtzeeGame::getClickedInventoryName(const Parser::Form& form, Common::Point cursor) {
	for (const auto& objName : items)
		if (isSelected<Parser::Image>(objName, form, cursor))
			return objName;

	return "";
}
*/



} // End of namespace Yahtzee
