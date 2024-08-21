#include "ui/gl/Widget.h"

namespace Game3 {
	void Widget::render(UIContext &, RendererContext &, float x, float y) {
		lastX = x;
		lastY = y;
	}
}
