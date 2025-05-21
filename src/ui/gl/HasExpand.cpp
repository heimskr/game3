#include "ui/gl/HasExpand.h"

namespace Game3 {
	HasExpand::HasExpand(Expansion horizontal, Expansion vertical):
		horizontalExpand(horizontal), verticalExpand(vertical) {}

	HasExpand::HasExpand() = default;

	Expansion HasExpand::getHorizontalExpand() const {
		return horizontalExpand;
	}

	Expansion HasExpand::getVerticalExpand() const {
		return verticalExpand;
	}

	Expansion HasExpand::getExpand(Orientation orientation) const {
		return orientation == Orientation::Horizontal? getHorizontalExpand() : getVerticalExpand();
	}

	void HasExpand::setHorizontalExpand(Expansion expand) {
		horizontalExpand = expand;
	}

	void HasExpand::setVerticalExpand(Expansion expand) {
		verticalExpand = expand;
	}

	void HasExpand::setExpand(Expansion horizontal, Expansion vertical) {
		setHorizontalExpand(horizontal);
		setVerticalExpand(vertical);
	}

	void HasExpand::setHorizontalExpand(bool expand) {
		setHorizontalExpand(expand? Expansion::Expand : Expansion::None);
	}

	void HasExpand::setVerticalExpand(bool expand) {
		setVerticalExpand(expand? Expansion::Expand : Expansion::None);
	}

	void HasExpand::setExpand(bool horizontal, bool vertical) {
		setHorizontalExpand(horizontal);
		setVerticalExpand(vertical);
	}
}
