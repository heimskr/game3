#include "graphics/Rectangle.h"
#include "ui/gl/widget/Widget.h"

namespace Game3 {
	Rectangle Widget::getLastRectangle() const {
		return {lastX, lastY, lastWidth, lastHeight};
	}

	void Widget::render(UIContext &, RendererContext &, float x, float y, float width, float height) {
		lastX = x;
		lastY = y;
		lastWidth = width;
		lastHeight = height;
	}

	std::shared_ptr<Widget> Widget::getDragStartWidget() {
		return nullptr;
	}

	bool Widget::click(UIContext &, int, int) {
		return false;
	}
}
