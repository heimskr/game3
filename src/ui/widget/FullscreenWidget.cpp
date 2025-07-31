#include "ui/widget/FullscreenWidget.h"

namespace Game3 {
	SizeRequestMode FullscreenWidget::getRequestMode() const {
		return SizeRequestMode::ConstantSize;
	}

	void FullscreenWidget::measure(const RendererContext &, Orientation, float, float, float &minimum, float &natural) {
		minimum = natural = -1;
	}
}
