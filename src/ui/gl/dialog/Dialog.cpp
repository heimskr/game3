#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "ui/gl/dialog/Dialog.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	Dialog::Dialog(UIContext &ui):
		Widget(ui, UI_SCALE) {}

	SizeRequestMode Dialog::getRequestMode() const {
		return SizeRequestMode::ConstantSize;
	}

	void Dialog::measure(const RendererContext &, Orientation orientation, float, float, float &minimum, float &natural) {
		const Rectangle position = getPosition();
		minimum = natural = orientation == Orientation::Horizontal? position.width : position.height;
	}

	void Dialog::onClose() {
		signalClose();
	}

	bool Dialog::hidesHotbar() const {
		return false;
	}

	bool Dialog::click(int button, int x, int y) {
		if (contains(x, y)) {
			Widget::click(button, x, y);
			return true;
		}

		return false;
	}

	bool Dialog::mouseDown(int button, int x, int y) {
		if (contains(x, y)) {
			Widget::mouseDown(button, x, y);
			return true;
		}

		return false;
	}

	bool Dialog::mouseUp(int button, int x, int y) {
		if (contains(x, y)) {
			Widget::mouseUp(button, x, y);
			return true;
		}

		return false;
	}

	bool Dialog::dragStart(int x, int y) {
		if (contains(x, y)) {
			Widget::dragStart(x, y);
			return true;
		}

		return false;
	}

	bool Dialog::dragUpdate(int x, int y) {
		if (contains(x, y)) {
			Widget::dragUpdate(x, y);
			return true;
		}

		return false;
	}

	bool Dialog::dragEnd(int x, int y) {
		if (contains(x, y)) {
			Widget::dragEnd(x, y);
			return true;
		}

		return false;
	}

	bool Dialog::scroll(float x_delta, float y_delta, int x, int y) {
		if (contains(x, y)) {
			Widget::scroll(x_delta, y_delta, x, y);
			return true;
		}

		return false;
	}

	bool Dialog::contains(int x, int y) const {
		return getPosition().contains(x, y);
	}

	bool Dialog::isFocused() const {
		return ui.getFocusedDialog().get() == this;
	}

	DialogPtr Dialog::getSelf() {
		return std::static_pointer_cast<Dialog>(shared_from_this());
	}

	ConstDialogPtr Dialog::getSelf() const {
		return std::static_pointer_cast<const Dialog>(shared_from_this());
	}

	void Dialog::render(const RendererContext &, float, float, float, float) {
		throw std::logic_error("Dialogs must be rendered with only a RendererContext argument");
	}

	void Dialog::render(const RendererContext &, const Rectangle &) {
		throw std::logic_error("Dialogs must be rendered with only a RendererContext argument");
	}
}
