#include "ui/gl/HasFixedSize.h"

namespace Game3 {
	HasFixedSize::HasFixedSize(float fixed_width, float fixed_height):
		HasFixedWidth(fixed_width), HasFixedHeight(fixed_height) {}

	HasFixedSize::HasFixedSize(float fixed_size):
		HasFixedSize(fixed_size, fixed_size) {}

	HasFixedSize::HasFixedSize() = default;

	std::pair<float, float> HasFixedSize::getFixedSize() const {
		return {fixedWidth, fixedHeight};
	}

	void HasFixedSize::setFixedSize(float fixed_width, float fixed_height) {
		setFixedWidth(fixed_width);
		setFixedHeight(fixed_height);
	}

	void HasFixedSize::setFixedSize(float fixed_size) {
		setFixedSize(fixed_size, fixed_size);
	}

	void HasFixedSize::fixSizes(float &width, float &height) {
		if (width < 0 || 0 < fixedWidth)
			width = fixedWidth;

		if (height < 0 || 0 < fixedHeight)
			height = fixedHeight;
	}
}
