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

#ifndef YAHTZEE_YAHTZEE_H
#define YAHTZEE_YAHTZEE_H

#include "engines/engine.h"

#include "common/archive.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/platform.h"
#include "common/ptr.h"
#include "common/queue.h"
#include "common/stack.h"
#include "common/str.h"
#include "common/stream.h"

#include "audio/mixer.h"

#include "gui/message.h"

#include "graphics/font.h"

#include "yahtzee/config.h"
#include "yahtzee/util.h"

#include "image/bmp.h"

struct ADGameDescription;

namespace Parser {
class Form;
class Label;
class Image;
class Timer;
class TextBox;
class Fileloc;
class OptionButton;
class CommandButton;
}


namespace Yahtzee {

class Console;

static const byte MOUSECURSOR_SCI[] = {
	1,1,0,0,0,0,0,0,0,0,0,
	1,2,1,0,0,0,0,0,0,0,0,
	1,2,2,1,0,0,0,0,0,0,0,
	1,2,2,2,1,0,0,0,0,0,0,
	1,2,2,2,2,1,0,0,0,0,0,
	1,2,2,2,2,2,1,0,0,0,0,
	1,2,2,2,2,2,2,1,0,0,0,
	1,2,2,2,2,2,2,2,1,0,0,
	1,2,2,2,2,2,2,2,2,1,0,
	1,2,2,2,2,2,2,2,2,2,1,
	1,2,2,2,2,2,1,0,0,0,0,
	1,2,1,0,1,2,2,1,0,0,0,
	1,1,0,0,1,2,2,1,0,0,0,
	0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,0,1,2,2,1,0
};

static const byte cursorPalette[] = {
	0, 0, 0,           // Black / Transparent
	0x80, 0x80, 0x80,  // Gray
	0xff, 0xff, 0xff   // White
};

enum YahtzeeDebugChannels {
	kDebugGeneral = 1 << 0
};

enum YahtzeeChapters {
	START,
	CHOP_SUEY,
	LUBRICANT,
	STRESSED,
	INFIDEL,
	SNIFFER,
	UNTOWARD,
	CLIMAX
};



enum OpType {
	Dialog,
	Hide,
	Show
};

struct Op {
	Op(OpType op) : opCode(op) {};
	OpType opCode;
};

struct DialogOp : public Op {
	DialogOp(Common::String d, int t, Common::String a) : Op(Dialog), dialog(d), type(t), author(a) {};
	Common::String dialog;
	int type;
	Common::String author;
};

struct HideOp : public Op {
	HideOp(Common::String obj) : Op(Hide), objName(obj) {};
	Common::String objName;
};

struct ShowOp : public Op {
	ShowOp(Common::String obj) : Op(Show), objName(obj) {};
	Common::String objName;
};

using OpList = Common::Array<Op*>;
using OpMap = Common::HashMap<Common::String, OpList>;

struct YahtzeeObject {
	YahtzeeObject(const Common::String& objName, const Common::String& cpt, const Common::Rect rect, OpList lOps, OpList oOps, OpList sOps, Common::HashMap<Common::String, OpList> uOps) :
		id(objName), caption(cpt), area(rect), lookOps(lOps), operateOps(oOps), speakOps(sOps), useOps(uOps) {};
	Common::String id;
	Common::String caption;
	Common::Rect area;
	OpList lookOps;
	OpList operateOps;
	OpList speakOps;
	Common::HashMap<Common::String, OpList> useOps;
};

using HashRect = Common::HashMap<Common::String, Common::Rect>;
using HashObj = Common::HashMap<Common::String, YahtzeeObject*>;

struct TimedOp {
	int32 time;
	OpList ops;
};

struct YahtzeeTimer {
	Common::Queue<TimedOp> ops;
};

const Graphics::PixelFormat PIXEL_FORMAT_16_BIT(2, 5, 5, 5, 0, 10, 5, 0, 0);
const Graphics::PixelFormat PIXEL_FORMAT_24_BIT(4, 8, 8, 8, 8, 8, 16, 24, 0);

class YahtzeeGame : public Engine {
public:
	YahtzeeGame(OSystem *syst, const ADGameDescription *gameDesc);
	~YahtzeeGame() override;

	Common::Error run() override;

	// Detection related functions
	const ADGameDescription *_gameDescription;
	const char *getGameId() const;
	Common::Platform getPlatform() const;

	Common::HashMap<Common::String, Parser::Form*> forms;
	Common::HashMap<Common::String, Graphics::Font*> fonts;

	Parser::Form* introForm;
	Parser::Form* mainMenuForm;
	Parser::Form* passwordForm;
	Parser::Form* inventoryForm;

	Parser::Form* currentForm;

	void launchPasswordScreen();

private:

	Image::BitmapDecoder* _image;
	Common::ScopedPtr<Console> _console;

	bool _timerInstalled; 


	enum Action {
		Redraw,
		ShowScene,
		UpdateScene,
		ChangeScene,
		PlaySound,
		DisplayMsg
	};

	enum YahtzeeAction {
		yNone,
		yLook,
		yOperate,
		ySpeak,
		yUse
	};

	YahtzeeAction currentAction;

	int _count;

	Common::Array<Common::String> _introScenes;
	Common::Array<Common::String> _chapterOne;
	Common::Array<Common::String> _chapterTwo;
	Common::Array<Common::String> _chapterThree;
	Common::Array<Common::String> _chapterFour;
	Common::Array<Common::String> _chapterFive;
	Common::Array<Common::String> _chapterSix;
	Common::Array<Common::String> _chapterSeven;
	Common::Array<Common::String> _chapterEight;

	uint _currentScene;
	Common::Queue<Action> _actions;

	uint _interval;
	GUI::MessageDialog* _md;
	Common::Queue<Common::String> msgQueue;

	Common::Rect bottomMenuQuit;
	Common::Rect bottomMenuLook;
	Common::Rect bottomMenuOperate;
	Common::Rect bottomMenuSpeakTo;
	Common::Rect bottomMenuUse;

	bool inventoryActive;
	bool sceneActive;

	void loadFonts();
	void loadImage(const Common::String &dirname, const Common::String &filename);
	void drawScreen();

	Audio::SoundHandle _soundHandle;

	void playSound();
	void stopSound();

	void showScene();
	void hideScene();

	void updateScene();
	void changeScene();

	void processTimer();
	static void onTimer(void *arg);

	void initTables();
	void readTables(const Common::String &fileName);
	void readForm(const Common::String &fileName);

	void readData();

	Common::String selectCommCaption(YahtzeeAction);

	Parser::Form* readForm(Common::SeekableReadStream &stream);
	Parser::Image* readImage(Common::SeekableReadStream &stream);
	Parser::Timer* readTimer(Common::SeekableReadStream &stream);
	Parser::TextBox* readTextbox(Common::SeekableReadStream &stream);
	Parser::CommandButton* readCommandButton(Common::SeekableReadStream &stream);
	Parser::OptionButton* readOptionButton(Common::SeekableReadStream &stream);

	void processKeyPress(Common::KeyCode);


	void initScreen();
	void initInventory();

	void updateForm(Parser::Form* form);
	void playIntro();
	void firstScene();
	void showInventory();
	void hideInventory();

	void previousScene();
	void nextScene();

	void displayMessage(const Common::String& msg);

	Graphics::Font* loadWinFont(const Common::String& fontFileName, const Common::String& fontName, double fontSize);

	void updateComm();
	void clearComm();

	void updateItem();
	void clearItem();

	void updateUseItem();
	void clearUseItem();

	void suspendTimer();
	void restoreTimer();

	void actOnOp(Op&);

	template<typename F>
	void blockTimer(F func) {
		suspendTimer();
		(this->*func)();
		restoreTimer();
	}

	template<typename F>
	void blockScreen(F func) {
		auto screen = _system->lockScreen();
		(this->*func)(screen);
		_system->unlockScreen();
		_system->updateScreen();
	}

	Graphics::Font* commFont;
	uint32 commForeColor;
	Common::String commCaption;

	Graphics::Font* itemFont;
	uint32 itemForeColor;
	Common::String itemCaption;

	Common::String useItemCaption;

	const Common::String COMM_LABEL_FONT_FILE_NAME = MS_SERIF_BOLD_FONT_FILE_NAME;
	const Common::String ITEM_LABEL_FONT_FILE_NAME = MS_SERIF_BOLD_FONT_FILE_NAME;


	const Common::String BOTTOM_MENU_QUIT = "Image25";
	const Common::String BOTTOM_MENU_LOOK = "Image1";
	const Common::String BOTTOM_MENU_OPERATE = "Image2";
	const Common::String BOTTOM_MENU_SPEAK = "Image3";
	const Common::String BOTTOM_MENU_USE = "Image4";

	const Common::String COMM_CAPTION_LABEL = "comm";
	const Common::String ITEM_CAPTION_LABEL = "item";
	const Common::String USE_ITEM_CAPTION_LABEL = "useitem";

	const Common::String LOOK_AT_CAPTION = "LOOK AT";
	const Common::String OPERATE_CAPTION = "OPERATE";
	const Common::String SPEAK_CAPTION = "SPEAK";
	const Common::String USE_CAPTION = "USE";

	Common::Rect sceneArea;
	Common::Rect inventoryArea;
	Common::Rect actionArea;

	const int16 MAX_COLORS = 256;

	/*
	 *
	 * 80x10	385x10
	 * 80x55	285x55
	 */
	const int32 INVENTORY_X1 = 80;
	const int32 INVENTORY_Y1 = 10;
	const int32 INVENTORY_X2 = 376;
	const int32 INVENTORY_Y2 = 91;

	const int32 INVENTORY_W = 396;
	const int32 INVENTORY_H = 112;

	HashObj sceneObjects;
	HashObj inventoryObjects;

	const int32 ACTION_X1 = 8;
	const int32 ACTION_Y1 = 8;
	const int32 ACTION_X2 = 75;
	const int32 ACTION_Y2 = 100;

	template<class T>
	Common::Rect invRectFromControl(const T& obj) {
		return buildRect(*inventoryForm, obj);
	}

	template<class T>
	Common::Rect rectFromControl(const T& obj) {
		return buildRect(*currentForm, obj);
	}


	Common::String getClickedObjectName(const Parser::Form& form, Common::Point cursor);
	Common::String getClickedInventoryName(const Parser::Form& form, Common::Point cursor);

	YahtzeeObject* getClickedObject(const Parser::Form& form, const Common::Point& cursor);
	YahtzeeObject* getClickedInventoryObject(const Parser::Form& form, const Common::Point& cursor);

	YahtzeeObject* selectedObject;
	YahtzeeObject* useObject;

	YahtzeeTimer currentTimer;

	Common::Array<Op*> testOps;
};



} // End of namespace Yahtzee

#endif
