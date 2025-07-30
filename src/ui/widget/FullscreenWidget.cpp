#include "ui/widget/FullscreenWidget.h"

namespace Game3 {
	SizeRequestMode FullscreenWidget::getRequestMode() const {
		return SizeRequestMode::ConstantSize;
	}

	void FullscreenWidget::measure(const RendererContext &renderers, Orientation, float for_width, float for_height, float &minimum, float &natural) {
		minimum = natural = -1;
	}
}
