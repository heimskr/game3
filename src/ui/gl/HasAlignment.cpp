#include "ui/gl/HasAlignment.h"

namespace Game3 {
	HasAlignment::HasAlignment(Alignment vertical, Alignment horizontal):
		verticalAlignment(vertical), horizontalAlignment(horizontal) {}

	HasAlignment::HasAlignment() = default;

	Alignment HasAlignment::getVerticalAlignment() const {
		return verticalAlignment;
	}

	Alignment HasAlignment::getHorizontalAlignment() const {
		return horizontalAlignment;
	}

	void HasAlignment::setVerticalAlignment(Alignment alignment) {
		verticalAlignment = alignment;
	}

	void HasAlignment::setHorizontalAlignment(Alignment alignment) {
		horizontalAlignment = alignment;
	}
}
