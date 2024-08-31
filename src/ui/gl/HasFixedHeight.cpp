#include "ui/gl/HasFixedHeight.h"

namespace Game3 {
	HasFixedHeight::HasFixedHeight(float fixed_height):
		fixedHeight(fixed_height) {}

	float HasFixedHeight::getFixedHeight() const {
		return fixedHeight;
	}

	void HasFixedHeight::setFixedHeight(float new_fixed_height) {
		fixedHeight = new_fixed_height;
	}
}
