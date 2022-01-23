/*
 * forms.cpp
 *
 *  Created on: 17 May 2020
 *      Author: kevin
 */

#include "common/archive.h"
#include "common/file.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/ptr.h"
#include "common/str.h"
#include "common/str-array.h"
#include "common/substream.h"
#include "common/system.h"
#include "common/util.h"

#include "graphics/surface.h"
#include "graphics/font.h"
#include "graphics/fonts/ttf.h"
#include "graphics/fonts/winfont.h"
#include "graphics/fontman.h"

#include "yahtzee/gui/dialogs.h"
#include "yahtzee/img/vbbmp.h"
#include "yahtzee/controls.h"
#include "yahtzee/util.h"

#include "yahtzee/config.h"


namespace Parser {



/*
 * Control
 */
Control::Control() {

}

void Control::assignProperties(ControlPropertyHashMap assignments) {}

Control::~Control() {}



/*
 * PositionedControl
 */
PositionedControl::PositionedControl() : Control(), left(0), top(0) {}

PositionedControl::PositionedControl(ControlPropertyHashMap assignments) : PositionedControl() {
	assignProperties(assignments);
}

void PositionedControl::assignProperties(ControlPropertyHashMap assignments) {
	assignProperty(left, LEFT, assignments);
	assignProperty(top, TOP, assignments);

	left = picaToPixels(left);
	top = picaToPixels(top);
}



/*
 * VisibleControl
 */
VisibleControl::VisibleControl() : PositionedControl(), height(0), width(0), visible(true) {}

VisibleControl::VisibleControl(ControlPropertyHashMap assignments) : VisibleControl() {
	assignProperties(assignments);
}

void VisibleControl::assignProperties(ControlPropertyHashMap assignments) {
	PositionedControl::assignProperties(assignments);

	assignProperty(height, HEIGHT, assignments);
	assignProperty(width, WIDTH, assignments);
	assignProperty(visible, VISIBLE, assignments);

	height = picaToPixels(height);
	width = picaToPixels(width);
}

VisibleControl::~VisibleControl() {}



/*
 * TabbedControl
 */
TabbedControl::TabbedControl() : VisibleControl(), tabIndex(0) {}

TabbedControl::TabbedControl(ControlPropertyHashMap assignments) : TabbedControl() {
	assignProperties(assignments);
}

void TabbedControl::assignProperties(ControlPropertyHashMap assignments) {
	TabbedControl::assignProperties(assignments);

	assignProperty(tabIndex, TAB_INDEX, assignments);
}

TabbedControl::~TabbedControl() {}



/*
 * CaptionedControl
 */
CaptionedControl::CaptionedControl() : VisibleControl(), caption("") {}

CaptionedControl::CaptionedControl(ControlPropertyHashMap assignments) : CaptionedControl() {
	assignProperties(assignments);
}

void CaptionedControl::assignProperties(ControlPropertyHashMap assignments) {
	VisibleControl::assignProperties(assignments);

	assignProperty(caption, CAPTION, assignments);
}

CaptionedControl::~CaptionedControl() {}



/*
 * ColoredControl
 */
ColoredControl::ColoredControl() : VisibleControl(), backColor(0x0000), foreColor(0x000) {}

ColoredControl::ColoredControl(ControlPropertyHashMap assignments) : ColoredControl() {
	assignProperties(assignments);
}

void ColoredControl::assignProperties(ControlPropertyHashMap assignments) {
	VisibleControl::assignProperties(assignments);

	assignProperty(backColor, BACK_COLOR, assignments);
	assignProperty(foreColor, FORE_COLOR, assignments);
}

ColoredControl::~ColoredControl() {}



/*
 * PictureControl
 */
PictureControl::PictureControl() : VisibleControl(), picture() {}

PictureControl::PictureControl(ControlPropertyHashMap assignments) : PictureControl() {
	assignProperties(assignments);
}

void PictureControl::assignProperties(ControlPropertyHashMap assignments) {
	VisibleControl::assignProperties(assignments);

	assignProperty(picture, PICTURE, assignments);
}

PictureControl::~PictureControl() {}



/*
 * Image
 */
Image::Image() : PictureControl() {}

Image::Image(ControlPropertyHashMap assignments) : Image() {
	PictureControl::assignProperties(assignments);
}

Image::~Image() {}



/*
 * Form
 */
Form::Form() : CaptionedControl(), ColoredControl(), PictureControl(), VisibleControl(),
		borderStyle(0),
		clientHeight(0),
		clientLeft(0),
		clientTop(0),
		clientWidth(0),
		linkTopic(""),
		scaleHeight(0),
		scaleWidth(0) {
}

Form::Form(ControlPropertyHashMap assignments) : Form() {
	assignProperties(assignments);
}

void Form::assignProperties(ControlPropertyHashMap assignments) {
	CaptionedControl::assignProperties(assignments);
	ColoredControl::assignProperties(assignments);
	PictureControl::assignProperties(assignments);
	VisibleControl::assignProperties(assignments);

	assignProperty(borderStyle, BORDER_STYLE, assignments);
	assignProperty(clientHeight, CLIENT_HEIGHT, assignments);
	assignProperty(clientLeft, CLIENT_LEFT, assignments);
	assignProperty(clientTop, CLIENT_TOP, assignments);
	assignProperty(clientWidth, CLIENT_WIDTH, assignments);
	assignProperty(linkTopic, LINK_TOPIC, assignments);
	assignProperty(scaleHeight, SCALE_HEIGHT, assignments);
	assignProperty(scaleWidth, SCALE_WIDTH, assignments);

	clientHeight = picaToPixels(clientHeight);
	clientLeft = picaToPixels(clientLeft);
	clientTop = picaToPixels(clientTop);
	clientWidth = picaToPixels(clientWidth);

	Common::Hash<Common::String> type_hash;

	AssignOpBase* aob = nullptr;
	AssignOp<Control*>* aocp = nullptr;
	Common::String property;

	for (auto& it : assignments) {
		aob = it._value;
		aocp = static_cast<AssignOp<Control*>*>(aob);
		property = aocp->property;

		switch (type_hash(aob->type)) {
		case IMAGE:
			images[property] = dynamic_cast<Image*>(aocp->value);
			break;
		case LABEL:
			labels[property] = dynamic_cast<Label*>(aocp->value);
			break;
		case TIMER:
			timers[property] = dynamic_cast<Timer*>(aocp->value);
			break;
		case COMMAND_BUTTON:
			commandButtons[property] = dynamic_cast<CommandButton*>(aocp->value);
			break;
		case OPTION_BUTTON:
			optionButtons[property] = dynamic_cast<OptionButton*>(aocp->value);
			break;
		case TEXTBOX:
			textboxes[property] = dynamic_cast<TextBox*>(aocp->value);
			break;
		default:
			controls[property] = aocp->value;
			break;
		}
	}
}

Form::~Form() {}

Parser::Control* Form::operator[](const char* idx) {
	return controls[idx];
}



/*
 * Label
 */
Label::Label() : CaptionedControl(), TabbedControl(), ColoredControl(), VisibleControl(),
		alignment(0),
		autoSize(false),
		backStyle(0),
		fontBold(false),
		fontItalic(false),
		fontName(""),
		fontSize(0),
		fontStrikeThru(false),
		fontUnderline(false) {
}

Label::Label(ControlPropertyHashMap assignments) : Label() {
	assignProperties(assignments);
}

void Label::assignProperties(ControlPropertyHashMap assignments) {
	CaptionedControl::assignProperties(assignments);
	TabbedControl::assignProperties(assignments);
	ColoredControl::assignProperties(assignments);

	assignProperty(alignment, ALIGNMENT, assignments);
	assignProperty(autoSize, AUTOSIZE, assignments);
	assignProperty(backStyle, BACK_STYLE, assignments);
	assignProperty(fontBold, FONT_BOLD, assignments);
	assignProperty(fontItalic, FONT_ITALIC, assignments);
	assignProperty(fontName, FONT_NAME, assignments);
	assignProperty(fontSize, FONT_SIZE, assignments);
	assignProperty(fontStrikeThru, FONT_STRIKETHRU, assignments);
	assignProperty(fontUnderline, FONT_UNDERLINE, assignments);
}

Label::~Label() {}



/*
 * TextBox
 */
TextBox::TextBox() : TabbedControl(), VisibleControl(), passwordChar('*') {}

TextBox::TextBox(ControlPropertyHashMap assignments) : TextBox() {
	assignProperties(assignments);
}

void TextBox::assignProperties(ControlPropertyHashMap assignments) {
	TabbedControl::assignProperties(assignments);

	assignProperty(passwordChar, PASSWORD_CHAR, assignments);
}

TextBox::~TextBox() {}



/*
 * Button
 */
Button::Button() : TabbedControl(), CaptionedControl(), VisibleControl() {}

Button::Button(ControlPropertyHashMap assignments) : Button() {
	assignProperties(assignments);
}

void Button::assignProperties(ControlPropertyHashMap assignments) {
	TabbedControl::assignProperties(assignments);
	CaptionedControl::assignProperties(assignments);
}

Button::~Button() {}



/*
 * CommandButton
 */
CommandButton::CommandButton() : Button(), VisibleControl() {}

CommandButton::CommandButton(ControlPropertyHashMap assignments) : CommandButton() {
	assignProperties(assignments);
}

void CommandButton::assignProperties(ControlPropertyHashMap assignments) {
	Button::assignProperties(assignments);
}

CommandButton::~CommandButton() {}



/*
 * OptionButton
 */
OptionButton::OptionButton() : Button(), ColoredControl(), VisibleControl() {}

OptionButton::OptionButton(ControlPropertyHashMap assignments) : OptionButton() {
	assignProperties(assignments);
}

void OptionButton::assignProperties(ControlPropertyHashMap assignments) {
	Button::assignProperties(assignments);
	ColoredControl::assignProperties(assignments);
}

OptionButton::~OptionButton() {}



/*
 * Line
 */
Line::Line() : _x1(0), _x2(0), _y1(0), _y2(0) {}

Line::Line(ControlPropertyHashMap assignments) : Line() {
	assignProperty(_x1, X1, assignments);
	assignProperty(_x2, X2, assignments);
	assignProperty(_y1, Y1, assignments);
	assignProperty(_y2, Y2, assignments);

	_x1 = picaToPixels(_x1);
	_x2 = picaToPixels(_x2);

	_y1 = picaToPixels(_y1);
	_y2 = picaToPixels(_y2);
}

Line::~Line() {}



/*
 * Timer
 */
Timer::Timer() : PositionedControl(), interval(0) {}

Timer::Timer(ControlPropertyHashMap assignments) : Timer() {}

void Timer::assignProperties(ControlPropertyHashMap assignments) {
	PositionedControl::assignProperties(assignments);

	assignProperty(interval, INTERVAL, assignments);
}

Timer::~Timer() {}





template<class T>
T* read(Common::SeekableReadStream &stream) {
	T* t = new T();
	return t;
}

template<class T>
void readVisible(T* label, Common::SeekableReadStream &stream) {
	label->height = stream.readUint32BE();
	label->width = stream.readUint32BE();
	label->visible = stream.readByte();
}

template<class T>
void readTab(T* label, Common::SeekableReadStream &stream) {
	label->tabIndex = stream.readUint32BE();
}

template<class T>
void readPosition(T* label, Common::SeekableReadStream &stream) {
	label->top = stream.readUint32BE();
	label->left = stream.readUint32BE();
}

template<class T>
void readColor(T* label, Common::SeekableReadStream &stream) {
	label->backColor = stream.readUint32BE();
	label->foreColor = stream.readUint32BE();
}

template<class T>
void readCaption(T* label, Common::SeekableReadStream &stream) {
	char buf[1024];
	memset(buf, 0, 1024);
	uint32 len = stream.readUint32BE();
	stream.read(buf, len);
	label->caption = buf;
}


template<>
Parser::Label* read(Common::SeekableReadStream &stream) {
	auto label = new Parser::Label();
	readVisible<Parser::Label>(label, stream);
	readPosition<Parser::Label>(label, stream);
	readColor<Parser::Label>(label, stream);
	readTab<Parser::Label>(label, stream);
	readCaption<Parser::Label>(label, stream);
	label->backStyle = stream.readUint32BE();
	label->alignment = stream.readUint32BE();
	label->fontName = stream.readPascalString();
	label->fontSize = stream.readUint32BE();
	label->autoSize = stream.readByte();
	label->fontBold = stream.readByte();
	label->fontItalic = stream.readByte();
	label->fontStrikeThru = stream.readByte();
	label->fontUnderline = stream.readByte();
	return label;
}

template<>
Parser::Timer* read(Common::SeekableReadStream &stream) {
	auto timer = new Parser::Timer();
	readPosition<Parser::Timer>(timer, stream);
	timer->interval = stream.readUint32BE();
	return timer;
}

template<>
Parser::TextBox* read(Common::SeekableReadStream &stream) {
	auto textbox = new Parser::TextBox();
	readVisible<Parser::TextBox>(textbox, stream);
	readPosition<Parser::TextBox>(textbox, stream);
	readTab<Parser::TextBox>(textbox, stream);
	textbox->passwordChar = stream.readByte();
	return textbox;
}


Parser::Fileloc readPicture(Common::SeekableReadStream &stream) {
	Parser::Fileloc picture;
	picture.filename = Yahtzee::YAHTZEE_DATA_FILENAME;
	picture.location = stream.pos();

	auto file_size = stream.readUint32BE();
	stream.skip(file_size);

	return picture;
}


template<>
Parser::Image* read(Common::SeekableReadStream &stream) {
	auto image = new Parser::Image();
	readVisible(image, stream);
	readPosition(image, stream);

	image->picture = readPicture(stream);

	return image;
}



template<>
Parser::CommandButton* read(Common::SeekableReadStream &stream) {
	auto commandButton = new Parser::CommandButton();
	readVisible<Parser::CommandButton>(commandButton, stream);
	readPosition<Parser::CommandButton>(commandButton, stream);
	readTab<Parser::CommandButton>(commandButton, stream);
	readCaption<Parser::CommandButton>(commandButton, stream);
	return commandButton;
}

template<>
Parser::OptionButton* read(Common::SeekableReadStream &stream) {
	auto optionButton = new Parser::OptionButton();
	readVisible<Parser::OptionButton>(optionButton, stream);
	readPosition<Parser::OptionButton>(optionButton, stream);
	readTab<Parser::OptionButton>(optionButton, stream);
	readColor<Parser::OptionButton>(optionButton, stream);
	readCaption<Parser::OptionButton>(optionButton, stream);
	return optionButton;
}


template<class T>
Common::HashMap<Common::String, T*> readControls(Common::SeekableReadStream &stream) {
	auto numLabels = stream.readByte();
	Common::String varname;

	Common::HashMap<Common::String, T*> controls;
	for (decltype(numLabels) i = 0; i < numLabels; ++i) {
		varname = stream.readPascalString();
		controls[varname] = read<T>(stream);
	}

	return controls;
}

template<>
Parser::Form* read(Common::SeekableReadStream &stream) {
	auto form = new Parser::Form();
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


Common::HashMap<Common::String, Parser::Form*> readForms() {
	Common::FSNode path(ConfMan.get("path"));
	auto dir = new Common::FSDirectory(path);

	Common::ArchiveMemberList list;
	dir->listMatchingMembers(list, "*.FRM");
	Common::sort(list.begin(), list.end(), Common::ArchiveMemberListComparator());


	Common::File data;

	if (!data.open(Yahtzee::YAHTZEE_DATA_FILENAME))
		error("sReadForms(): Error reading DAT file");

	char buffer[4];
	data.read(&buffer, 4);

	if (memcmp(buffer, Yahtzee::FRIDAY_PREFIX.c_str(), 4))
		error("sReadForms(): Incorrect DAT format");
	return readControls<Parser::Form>(data);
}

}
