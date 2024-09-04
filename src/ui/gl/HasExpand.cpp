#include "ui/gl/HasExpand.h"

namespace Game3 {
	HasExpand::HasExpand(bool horizontal, bool vertical):
		horizontalExpand(horizontal), verticalExpand(vertical) {}

	HasExpand::HasExpand() = default;

	bool HasExpand::getHorizontalExpand() const {
		return horizontalExpand;
	}

	bool HasExpand::getVerticalExpand() const {
		return verticalExpand;
	}

	void HasExpand::setHorizontalExpand(bool expand) {
		horizontalExpand = expand;
	}

	void HasExpand::setVerticalExpand(bool expand) {
		verticalExpand = expand;
	}

	void HasExpand::setExpand(bool horizontal, bool vertical) {
		setHorizontalExpand(horizontal);
		setVerticalExpand(vertical);
	}
}
