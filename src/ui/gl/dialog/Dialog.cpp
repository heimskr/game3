#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "ui/gl/dialog/Dialog.h"
#include "ui/gl/Constants.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	Dialog::Dialog(UIContext &ui, float selfScale):
		Widget(ui, selfScale) {}

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

	bool Dialog::click(int button, int x, int y, Modifiers modifiers) {
		if (contains(x, y)) {
			return Widget::click(button, x, y, modifiers);
		}

		return false;
	}

	bool Dialog::mouseDown(int button, int x, int y, Modifiers modifiers) {
		if (contains(x, y)) {
			return Widget::mouseDown(button, x, y, modifiers);
		}

		return false;
	}

	bool Dialog::mouseUp(int button, int x, int y, Modifiers modifiers) {
		if (contains(x, y)) {
			return Widget::mouseUp(button, x, y, modifiers);
		}

		return false;
	}

	bool Dialog::dragStart(int x, int y) {
		if (contains(x, y)) {
			return Widget::dragStart(x, y);
		}

		return false;
	}

	bool Dialog::dragUpdate(int x, int y) {
		if (contains(x, y)) {
			return Widget::dragUpdate(x, y);
		}

		return false;
	}

	bool Dialog::dragEnd(int x, int y, double displacement) {
		if (contains(x, y)) {
			return Widget::dragEnd(x, y, displacement);
		}

		return false;
	}

	bool Dialog::scroll(float x_delta, float y_delta, int x, int y, Modifiers modifiers) {
		if (contains(x, y)) {
			return Widget::scroll(x_delta, y_delta, x, y, modifiers);
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
