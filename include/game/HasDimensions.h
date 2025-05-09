#pragma once

#include "math/Vector.h"
#include "types/Position.h"

namespace Game3 {
	class Realm;

	class HasDimensions {
		protected:
			HasDimensions() = default;

		public:
			HasDimensions(const HasDimensions &) = default;
			HasDimensions(HasDimensions &&) noexcept = default;

			virtual ~HasDimensions() = default;

			HasDimensions & operator=(const HasDimensions &) = default;
			HasDimensions & operator=(HasDimensions &&) noexcept = default;

			virtual Vector2i getDimensions() const;
			virtual Vector2i getAnchor() const;
			virtual Position getPosition() const = 0;

			bool occupies(const Position &) const;
			bool collidesAt(const Realm &, const Position &) const;

			/** Iterates over every occupied position until the function returns true. */
			template <typename Fn>
			void iterateTiles(const Fn &fn) const {
				const Position position = getPosition();
				const Vector2i dimensions = getDimensions();

				if (dimensions.x == 1 && dimensions.y == 1) {
					fn(position);
					return;
				}

				const Vector2i anchor = getAnchor();

				for (Index row = position.row - anchor.y; row < position.row - anchor.y + dimensions.y; ++row)
					for (Index column = position.column - anchor.x; column < position.column - anchor.x + dimensions.x; ++column)
						if (fn(Position(row, column)))
							return;
			}
	};
}
