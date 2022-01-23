
/*
 * forms.h
 *
 *  Created on: 17 May 2020
 *      Author: kevin
 */

#ifndef ENGINES_YAHTZEE_CONTROLS_H_
#define ENGINES_YAHTZEE_CONTROLS_H_


#include "common/list.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/ptr.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/str-array.h"
#include "common/util.h"

#include "yahtzee/yahtzee.h"

enum ControlType {
	FORM = 0x2c028a6,
	IMAGE = 0x532aa6ea,
	OPTION_BUTTON = 0xf99b7043,
	COMMAND_BUTTON = 0xede4cc52,
	LINE = 0xbe93f8ba,
	TEXTBOX = 0xe2ba72df,
	LABEL = 0x6f3b888f,
	TIMER = 0x57db76ce
};

enum ControlProperty {
	ALIGNMENT = 0x15ee2fe,
	AUTOSIZE = 0x4ae01262,
	BORDER_STYLE = 0xcf2eed9a,
	CAPTION = 0x97598bd1,
	CLIENT_HEIGHT = 0x8515760c,
	CLIENT_LEFT = 0x20faafc8,
	CLIENT_TOP = 0xc9c497b3,
	CLIENT_WIDTH = 0x13eae530,
	HEIGHT = 0xb5ffb16f,
	ICON = 0xb06f563f,
	LEFT = 0xc65d275f,
	LINK_TOPIC = 0xd998d50c,
	PICTURE = 0x628292a1,
	SCALE_HEIGHT = 0xf5ce92a0,
	SCALE_WIDTH = 0x8ae11980,
	TOP = 0x970d710a,
	WIDTH = 0xd6334c11,

	INTERVAL = 0xc405534b,
	VISIBLE = 0x53cd82f1,
	BACK_STYLE = 0x97bfea9d,
	BACK_COLOR = 0xf6b3c8b7,
	FORE_COLOR = 0xf96b3832,
	TAB_INDEX = 0xc65d275f,

	FONT_BOLD = 0xfe85f7a2,
	FONT_ITALIC = 0xe1e8c3d,
	FONT_NAME = 0xeea61722,
	FONT_SIZE = 0xcab76054,
	FONT_STRIKETHRU = 0xbc5401f8,
	FONT_UNDERLINE = 0x8e11ba80,

	PASSWORD_CHAR,
	X1,
	X2,
	Y1,
	Y2
};


namespace Common {
template<>
struct Hash<ControlProperty> {
	uint operator()(const ControlProperty &cp) const {
		return cp + (cp >> 3);
	}
};
}

namespace Parser {

enum Font {
	MS_SERIF,
	MS_SANS_SERIF,
	TIMES_NEW_ROMAN
};

using color_hex = int32;

struct AssignOpBase {
	AssignOpBase(Common::String t, Common::String prop) : type(t), property(prop) {
	}
	Common::String type;
	Common::String property;
};

using ControlPropertyHashMap = Common::HashMap<ControlProperty, AssignOpBase*>;


template <typename T>
struct AssignOp : AssignOpBase {
	AssignOp(Common::String t, Common::String prop, T val) : AssignOpBase(t, prop), value(val) {
	}
	T value;
};

struct Fileloc {
	Fileloc() {
		filename = "";
		location = 0;
	}
	Fileloc(Common::String f, uint loc) {
		filename = f;
		location = loc;
	}
	Common::String filename;
	uint location;
};


template<typename T>
inline T to(Common::StringArray val) {
	return val[0];
}

template<>
inline double to(Common::StringArray val) {
	char* endptr;
	return strtod(val[0].c_str(), &endptr);
}

template<>
inline long to(Common::StringArray val) {
	char* endptr;
	if (val.size() > 1) {
		int base = strtol(val[1].c_str(), &endptr, 10);
		return strtol(val[0].c_str(), &endptr, base);
	}
	return strtol(val[0].c_str(), &endptr, 10);
}

template<>
inline int to(Common::StringArray val) {
	return to<long>(val);
}

template<>
inline uint to(Common::StringArray val) {
	return to<long>(val);
}

template<>
inline char to(Common::StringArray val) {
	return val[0].firstChar();
}

template<>
inline bool to(Common::StringArray val) {
	bool b;
	parseBool(val[0], b);
	return b;
}

template<>
inline Fileloc to(Common::StringArray val) {
	char* endptr;
	int location = strtol(val[1].c_str(), &endptr, 16);
	return Fileloc(val[0], location);
}


class Control {
public:
	Control();

	virtual ~Control() = 0;
protected:
	virtual void assignProperties(ControlPropertyHashMap);

	template<typename T>
	inline void assignProperty(T& prop, ControlProperty cp, ControlPropertyHashMap& props) {
		AssignOpBase* aob = props.getVal(cp);
		if (aob) {
			AssignOp<Common::StringArray>* ao = static_cast<AssignOp<Common::StringArray>*>(aob);
			prop = to<T>(ao->value);
			props.erase(cp);
		}
	}

/*
	inline void assignProperty(double&, ControlProperty, Common::HashMap<ControlProperty, AssignOpBase*>&);
	inline void assignProperty(int&, ControlProperty, Common::HashMap<ControlProperty, AssignOpBase*>&);
	inline void assignProperty(uint&, ControlProperty, Common::HashMap<ControlProperty, AssignOpBase*>&);
	inline void assignProperty(bool&, ControlProperty, Common::HashMap<ControlProperty, AssignOpBase*>&);
	inline void assignProperty(char&, ControlProperty, Common::HashMap<ControlProperty, AssignOpBase*>&);
	inline void assignProperty(Common::String&, ControlProperty, Common::HashMap<ControlProperty, AssignOpBase*>&);
	inline void assignProperty(Parser::Fileloc&, ControlProperty, Common::HashMap<ControlProperty, AssignOpBase*>&);
*/
};

class PositionedControl : public Control {
public:
	PositionedControl();
	PositionedControl(ControlPropertyHashMap);
protected:
	virtual void assignProperties(ControlPropertyHashMap) override;
public:
	int32 left;
    int32 top;
};

class VisibleControl : public PositionedControl {
public:
	VisibleControl();
	VisibleControl(ControlPropertyHashMap);
	virtual ~VisibleControl() = 0;

	uint height;
	uint width;
	bool visible;
protected:
	virtual void assignProperties(ControlPropertyHashMap) override;
};

class TabbedControl : public virtual VisibleControl {
public:
	TabbedControl();
	TabbedControl(ControlPropertyHashMap);
	virtual ~TabbedControl() = 0;

	uint tabIndex;
protected:
	virtual void assignProperties(ControlPropertyHashMap) override;
};

class CaptionedControl : public virtual VisibleControl {
public:
	CaptionedControl();
	CaptionedControl(ControlPropertyHashMap);
	virtual ~CaptionedControl() = 0;

	Common::String caption;
protected:
	virtual void assignProperties(ControlPropertyHashMap) override;
};

class ColoredControl : public virtual VisibleControl {
public:
	ColoredControl();
	ColoredControl(ControlPropertyHashMap);
	virtual ~ColoredControl() = 0;

	uint32 backColor;
	uint32 foreColor;
protected:
	virtual void assignProperties(ControlPropertyHashMap) override;
};

class PictureControl : public virtual VisibleControl {
public:
	PictureControl();
	PictureControl(ControlPropertyHashMap);
	virtual ~PictureControl() = 0;

	Fileloc picture;
protected:
	virtual void assignProperties(ControlPropertyHashMap) override;
};

class Image : public PictureControl {
public:
	Image();
	Image(ControlPropertyHashMap);
	~Image();
};

class Label : public TabbedControl, public CaptionedControl, public ColoredControl {
public:
	Label();
	Label(ControlPropertyHashMap);
	~Label();

	uint alignment;
	bool autoSize;
	uint backStyle;
	bool fontBold;
	bool fontItalic;
	Common::String fontName;
	double fontSize;
	bool fontStrikeThru;
	bool fontUnderline;

protected:
	virtual void assignProperties(ControlPropertyHashMap) override;
};

class Timer : public PositionedControl {
public:
	Timer();
	Timer(ControlPropertyHashMap);
	~Timer();

	uint interval;

protected:
	virtual void assignProperties(ControlPropertyHashMap) override;
};

class TextBox : public TabbedControl {
public:
	TextBox();
	TextBox(ControlPropertyHashMap);
	~TextBox();

	char passwordChar;
protected:
	virtual void assignProperties(ControlPropertyHashMap) override;
};

class Button : public TabbedControl, public CaptionedControl {
public:
	Button();
	Button(ControlPropertyHashMap);
	virtual ~Button() = 0;
private:

protected:
	void assignProperties(ControlPropertyHashMap);
};

class CommandButton : public Button {
public:
	CommandButton();
	CommandButton(ControlPropertyHashMap);
	~CommandButton();
protected:
	void assignProperties(ControlPropertyHashMap);
};

class OptionButton : public Button, public ColoredControl {
public:
	OptionButton();
	OptionButton(ControlPropertyHashMap);
	~OptionButton();
protected:
	void assignProperties(ControlPropertyHashMap);
};

class Form : public CaptionedControl, public ColoredControl, public PictureControl {
public:
	Form();
	Form(ControlPropertyHashMap);
	~Form();


public:
	uint borderStyle;
	uint clientHeight;
	uint clientLeft;
	uint clientTop;
	uint clientWidth;
	Fileloc icon;
	Common::String linkTopic;
	uint scaleHeight;
	uint scaleWidth;

	Control* operator[](const char*);
	Label* getLabel(const char*);
	Image* getImage(const char*);


	Common::HashMap<Common::String, Image*> images;
	Common::HashMap<Common::String, Label*> labels;
	Common::HashMap<Common::String, Timer*> timers;
	Common::HashMap<Common::String, OptionButton*> optionButtons;
	Common::HashMap<Common::String, CommandButton*> commandButtons;
	Common::HashMap<Common::String, TextBox*> textboxes;
	Common::HashMap<Common::String, Control*> controls;

protected:
	virtual void assignProperties(ControlPropertyHashMap) override;
};



class Line : public Control {
public:
	Line();
	Line(ControlPropertyHashMap);
	~Line();

	int32 _x1, _x2;
	int32 _y1, _y2;
};


Parser::Label* readLabel(Common::SeekableReadStream &stream);
void readTab(Parser::Label* label, Common::SeekableReadStream &stream);
void readPosition(Parser::Label* label, Common::SeekableReadStream &stream);
void readColor(Parser::Label* label, Common::SeekableReadStream &stream);
void readCaption(Parser::Label* label, Common::SeekableReadStream &stream);

Common::HashMap<Common::String, Parser::Form*> readForms();






}
#endif /* ENGINES_YAHTZEE_CONTROLS_H_ */


