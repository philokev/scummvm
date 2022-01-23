/*
 * widget.cpp
 *
 *  Created on: Dec 12, 2021
 *      Author: kevinlaurier
 */

#include "gui/dialog.h"
#include "gui/gui-manager.h"
#include "gui/widgets/edittext.h"
#include "gui/widget.h"

#include "yahtzee/util.h"
#include "yahtzee/gui/widget.h"

namespace Yahtzee {

ContainerImageWidget::ContainerImageWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String& tooltip) :
		GUI::GraphicsWidget(boss, x, y, w, h, tooltip) {
}

ContainerImageWidget::ContainerImageWidget(GuiObject *boss, const Common::String &name, const Common::U32String& tooltip) :
		GUI::GraphicsWidget(boss, name, tooltip) {
}

ContainerImageWidget::~ContainerImageWidget() {
	_gfx.free();
	for (auto w = _firstWidget; w; w = w->next()) {
		_boss->removeWidget(w);
	}
}

bool ContainerImageWidget::containsWidget(GUI::Widget *w) const {
	return containsWidgetInChain(_firstWidget, w);
}

GUI::Widget *ContainerImageWidget::findWidget(int x, int y) {
	auto widget = findWidgetInChain(_firstWidget, x, y);
	if (widget)
		return widget;
	return this;
}

void ContainerImageWidget::removeWidget(GUI::Widget *widget) {
	// We also remove the widget from the boss to avoid a reference to a
	// widget not in the widget chain anymore.
	_boss->removeWidget(widget);

	Widget::removeWidget(widget);
}

ClickableImageWidget::ClickableImageWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String& tooltip, uint32 cmd) :
		GUI::GraphicsWidget(boss, x, y, w, h, tooltip), GUI::CommandSender(boss), _cmd(cmd) {
}

ClickableImageWidget::ClickableImageWidget(GuiObject *boss, const Common::String &name, const Common::U32String& tooltip, uint32 cmd) :
		GUI::GraphicsWidget(boss, name, tooltip), GUI::CommandSender(boss), _cmd(cmd) {
}

ClickableImageWidget::~ClickableImageWidget() {
}

void ClickableImageWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	if (isEnabled() && x >= 0 && x < _w && y >= 0 && y < _h) {
		sendCommand(_cmd, 0);
	}
}


PasswordTextWidget::PasswordTextWidget(GuiObject *boss, int x, int y, int w, int h, const Common::String &text, const Common::U32String& tooltip, uint32 cmd, uint32 finishCmd, char replaceChar) :
	EditTextWidget(boss, x, y, w, h, text, tooltip, cmd, finishCmd), _replaceChar(replaceChar) {
}


PasswordTextWidget::PasswordTextWidget(GuiObject *boss, const Common::String &name, const Common::String &text, const Common::U32String& tooltip, uint32 cmd, uint32 finishCmd, char replaceChar) :
	EditTextWidget(boss, name, text, tooltip, cmd, finishCmd), _replaceChar(replaceChar) {
}

PasswordTextWidget::~PasswordTextWidget() {

}


bool PasswordTextWidget::tryInsertChar(byte c, int pos) {
	_password.insertChar(c, pos);
	_editString.insertChar(_replaceChar, pos);
	return true;
}

void writeLabel(Graphics::Surface& surface, const Parser::Label& label, const Graphics::Font& font) {
	auto foreColor = msColorToRGB(g_system->getScreenFormat(), label.foreColor);
	Common::StringArray paragraph;
	font.wordWrapText(label.caption, label.width, paragraph, 0, false);
	uint lineHeight = font.getFontHeight();
	uint y = 0;

	for (auto& line : paragraph) {
		font.drawString(&surface, line, 0, y, label.width, foreColor, Graphics::kTextAlignCenter);
		y += lineHeight;
	}
}

void writeSurface(Graphics::Surface& surface, uint width, uint height) {
	surface.create(width, height, g_system->getScreenFormat());
}

}
