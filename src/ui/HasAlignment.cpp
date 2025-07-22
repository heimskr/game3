#include "ui/HasAlignment.h"

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

	Alignment HasAlignment::getAlignment(Orientation orientation) const {
		return orientation == Orientation::Horizontal? getHorizontalAlignment() : getVerticalAlignment();
	}

	void HasAlignment::setHorizontalAlignment(Alignment alignment) {
		horizontalAlignment = alignment;
	}

	void HasAlignment::setVerticalAlignment(Alignment alignment) {
		verticalAlignment = alignment;
	}

	void HasAlignment::setAlignment(Alignment alignment) {
		setHorizontalAlignment(alignment);
		setVerticalAlignment(alignment);
	}

	void HasAlignment::setAlignment(Orientation orientation, Alignment alignment) {
		if (orientation == Orientation::Horizontal)
			setHorizontalAlignment(alignment);
		else
			setVerticalAlignment(alignment);
	}

	void HasAlignment::setAlignment(Alignment horizontal, Alignment vertical) {
		setHorizontalAlignment(horizontal);
		setVerticalAlignment(vertical);
	}

	void HasAlignment::adjustCoordinate(Orientation orientation, float &coordinate, float available_size, float widget_size) {
		if (available_size < 0)
			return;

		switch (getAlignment(orientation)) {
			case Alignment::Start:
				break;

			case Alignment::Center:
				coordinate += (available_size - widget_size) / 2;
				break;

			case Alignment::End:
				coordinate += available_size - widget_size;
				break;

			default:
				break;
		}
	}
}
