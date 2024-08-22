#include "graphics/Rectangle.h"
#include "ui/gl/Widget.h"

namespace Game3 {
	Rectangle Widget::getLastRectangle() const {
		return {lastX, lastY, lastWidth, lastHeight};
	}

	void Widget::render(UIContext &, RendererContext &, float x, float y) {
		lastX = x;
		lastY = y;
	}

	std::shared_ptr<Widget> Widget::getDragStartWidget() {
		return nullptr;
	}

	bool Widget::click(UIContext &) {
		return false;
	}
}
