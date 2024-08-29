#include "graphics/Rectangle.h"
#include "ui/gl/widget/Widget.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	Rectangle Widget::getLastRectangle() const {
		return {lastX, lastY, lastWidth, lastHeight};
	}

	void Widget::render(UIContext &ui, const RendererContext &, float x, float y, float width, float height) {
		const Rectangle top = ui.scissorStack.getTop().rectangle;
		lastX = top.x + x;
		lastY = top.y + y;
		lastWidth = width;
		lastHeight = height;
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

	bool Widget::click(UIContext &, int, int, int) {
		return false;
	}

	bool Widget::dragStart(UIContext &, int, int) {
		return false;
	}

	bool Widget::dragUpdate(UIContext &, int, int) {
		return false;
	}

	bool Widget::dragEnd(UIContext &, int, int) {
		return false;
	}

	bool Widget::scroll(UIContext &, float, float, int, int) {
		return false;
	}
}
