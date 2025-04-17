#include "game/HasDimensions.h"
#include "realm/Realm.h"

namespace Game3 {
	Vector2i HasDimensions::getDimensions() const {
		return {1, 1};
	}

	Vector2i HasDimensions::getAnchor() const {
		return {0, 0};
	}

	bool HasDimensions::occupies(const Position &check) const {
		const auto [dimensions_x, dimensions_y] = getDimensions();

		if (dimensions_x == 1 && dimensions_y == 1) {
			return getPosition() == check;
		}

		const auto [row, column] = getPosition();
		const auto [anchor_x, anchor_y] = getAnchor();

		const auto x_min = column - anchor_x;
		const auto x_max = x_min + dimensions_x - 1;

		const auto y_min = row - anchor_y;
		const auto y_max = y_min + dimensions_y - 1;

		return y_min <= check.row && check.row <= y_max && x_min <= check.column && check.column <= x_max;
	}

	bool HasDimensions::collidesAt(const Realm &realm, const Position &position) const {
		return occupies(position) && !realm.isPathable(position);
	}
}
