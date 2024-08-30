#include "Log.h"
#include "graphics/Rectangle.h"
#include "ui/gl/widget/Widget.h"
#include "ui/gl/UIContext.h"
#include "util/Demangle.h"

namespace Game3 {
	Widget::Widget(float scale):
		scale(scale) {}

	const Rectangle & Widget::getLastRectangle() const {
		return lastRectangle;
	}

	void Widget::render(UIContext &ui, const RendererContext &, float x, float y, float width, float height) {
		lastRectangle = ui.scissorStack.getTop().rectangle + Rectangle(x, y, width, height);
	}

	void Widget::render(UIContext &ui, const RendererContext &renderers, const Rectangle &rectangle) {
		render(ui, renderers,
		       static_cast<float>(rectangle.x),
		       static_cast<float>(rectangle.y),
		       static_cast<float>(rectangle.width),
		       static_cast<float>(rectangle.height));
	}

	std::shared_ptr<Widget> Widget::getDragStartWidget() {
		return nullptr;
	}

	bool Widget::click(UIContext &ui, int button, int x, int y) {
		if (onClick)
			return onClick(*this, ui, button, x - lastRectangle.x, y - lastRectangle.y);

		return false;
	}

	bool Widget::dragStart(UIContext &, int, int) {
		return false;
	}

	bool Widget::dragUpdate(UIContext &ui, int x, int y) {
		if (onDrag)
			return onDrag(*this, ui, x - lastRectangle.x, y - lastRectangle.y);

		return false;
	}

	bool Widget::dragEnd(UIContext &, int, int) {
		return false;
	}

	bool Widget::scroll(UIContext &, float, float, int, int) {
		return false;
	}

	bool Widget::keyPressed(UIContext &, uint32_t, Modifiers) {
		return false;
	}

	float Widget::getScale() const {
		return scale;
	}

	void Widget::setOnClick(decltype(onClick) new_onclick) {
		onClick = std::move(new_onclick);
	}

	void Widget::setOnDrag(decltype(onDrag) new_ondrag) {
		onDrag = std::move(new_ondrag);
	}
}
