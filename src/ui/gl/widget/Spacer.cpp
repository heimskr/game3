#include "ui/gl/widget/Spacer.h"
#include "ui/gl/UIContext.h"
#include "util/Log.h"

namespace Game3 {
	Spacer::Spacer(UIContext &ui, Orientation orientation):
		Widget(ui, 1),
		orientation(orientation) {}

	void Spacer::init() {
		if (orientation == Orientation::Vertical) {
			setVerticalExpand(true);
		} else {
			setHorizontalExpand(true);
		}
	}

	void Spacer::render(const RendererContext &renderers, float x, float y, float width, float height) {
		Widget::render(renderers, x, y, width, height);
	}

	SizeRequestMode Spacer::getRequestMode() const {
		return SizeRequestMode::Expansive;
	}

	void Spacer::measure(const RendererContext &, Orientation measure_orientation, float for_width, float for_height, float &minimum, float &natural) {
		if (measure_orientation == Orientation::Horizontal) {
			minimum = 0;
			natural = horizontalExpand && 0 < for_width? for_width : 1;
		} else {
			minimum = 0;
			natural = verticalExpand && 0 < for_height? for_height : 1;
		}
	}
}
