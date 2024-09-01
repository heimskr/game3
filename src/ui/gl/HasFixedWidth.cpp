#include "ui/gl/HasFixedWidth.h"

namespace Game3 {
	HasFixedWidth::HasFixedWidth(float fixed_width):
		fixedWidth(fixed_width) {}

	HasFixedWidth::HasFixedWidth() = default;

	float HasFixedWidth::getFixedWidth() const {
		return fixedWidth;
	}

	void HasFixedWidth::setFixedWidth(float new_fixed_width) {
		fixedWidth = new_fixed_width;
	}
}
