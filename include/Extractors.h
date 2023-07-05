#include "Directions.h"

namespace Game3 {
	struct Extractors: Directions {
		using Directions::Directions;

		/** (x, y) */
		template <typename T = int>
		inline std::pair<T, T> getOffsets() const {
			const uint8_t mask(*this);
			return {mask % 8, mask / 8};
		}
	};
}
