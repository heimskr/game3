#include "ui/gl/HasAlignment.h"

namespace Game3 {
	HasAlignment::HasAlignment(Alignment horizontal, Alignment vertical):
		horizontalAlignment(horizontal), verticalAlignment(vertical) {}

	HasAlignment::HasAlignment() = default;

	Alignment HasAlignment::getHorizontalAlignment() const {
		return horizontalAlignment;
	}

	Alignment HasAlignment::getVerticalAlignment() const {
		return verticalAlignment;
	}

	void HasAlignment::setHorizontalAlignment(Alignment alignment) {
		horizontalAlignment = alignment;
	}

	void HasAlignment::setVerticalAlignment(Alignment alignment) {
		verticalAlignment = alignment;
	}

	void HasAlignment::setAlignment(Alignment horizontal, Alignment vertical) {
		setHorizontalAlignment(horizontal);
		setVerticalAlignment(vertical);
	}
}
