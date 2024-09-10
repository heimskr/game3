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

	Alignment HasAlignment::getAlignment(Orientation orientation) const {
		return orientation == Orientation::Horizontal? getHorizontalAlignment() : getVerticalAlignment();
	}

	void HasAlignment::setHorizontalAlignment(Alignment alignment) {
		horizontalAlignment = alignment;
	}

	void HasAlignment::setVerticalAlignment(Alignment alignment) {
		verticalAlignment = alignment;
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
		switch (getAlignment(orientation)) {
			case Alignment::Start:
				break;

			case Alignment::Middle:
				if (0 < available_size)
					coordinate += (available_size - widget_size) / 2;
				break;

			case Alignment::End:
				if (0 < available_size)
					coordinate += available_size - widget_size;
				break;

			default:
				break;
		}
	}
}
