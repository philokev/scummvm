/*
 * widget.h
 *
 *  Created on: Dec 12, 2021
 *      Author: kevinlaurier
 */

#include "gui/dialog.h"
#include "gui/gui-manager.h"
#include "gui/widget.h"
#include "graphics/font.h"

namespace Yahtzee {

class ContainerImageWidget : public GUI::GraphicsWidget {
public:
	ContainerImageWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String& tooltip = Common::U32String(""));
	ContainerImageWidget(GuiObject *boss, const Common::String &name, const Common::U32String& tooltip = Common::U32String(""));
	virtual ~ContainerImageWidget();

protected:
	bool containsWidget(Widget *) const override;
	Widget *findWidget(int x, int y) override;
	void removeWidget(Widget *widget) override;
};

using ContainerImageWidgetPtr = Common::ScopedPtr<ContainerImageWidget>;

/*
class ClickabkeArea : public GUI::GuiObject {
public:
	ClickableArea(int x,)
};
*/

class ClickableImageWidget : public GUI::GraphicsWidget, public GUI::CommandSender {
public:
	ClickableImageWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String& tooltip = Common::U32String(""), uint32 cmd = 0);
	ClickableImageWidget(GuiObject *boss, const Common::String &name, const Common::U32String& tooltip = Common::U32String(""), uint32 cmd = 0);
	virtual ~ClickableImageWidget();

	void handleMouseUp(int x, int y, int button, int clickCount) override;
protected:
	uint32	_cmd;
};

using ClickableImageWidgetPtr = Common::ScopedPtr<ClickableImageWidget>;

class PasswordTextWidget : public GUI::EditTextWidget {
public:
	PasswordTextWidget(GuiObject *boss, int x, int y, int w, int h, const Common::String &text, const Common::U32String& tooltip = Common::U32String(""), uint32 cmd = 0, uint32 finishCmd = 0, char replaceChar = '*');
	PasswordTextWidget(GuiObject *boss, const Common::String &name, const Common::String &text, const Common::U32String& tooltip = Common::U32String(""), uint32 cmd = 0, uint32 finishCmd = 0, char replaceChar = '*');
	virtual ~PasswordTextWidget();
protected:
	bool tryInsertChar(byte c, int pos) override;
	Common::String _password;
	char _replaceChar;
};

using PasswordTextWidgetPtr = Common::ScopedPtr<PasswordTextWidget>;

void writeLabel(Graphics::Surface&, const Parser::Label&, const Graphics::Font&);
void writeSurface(Graphics::Surface&, uint, uint);
}
