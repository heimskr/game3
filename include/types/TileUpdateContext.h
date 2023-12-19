#pragma once

#include <cstdint>

namespace Game3 {
	struct TileUpdateContext {
		uint32_t limit;

		TileUpdateContext();
		explicit TileUpdateContext(uint32_t);

		TileUpdateContext & operator--() {
			--limit;
			return *this;
		}

		TileUpdateContext operator--(int) {
			return TileUpdateContext(limit--);
		}

		TileUpdateContext operator-(int to_subtract) {
			return TileUpdateContext(limit - to_subtract);
		}

		TileUpdateContext & operator-=(int to_subtract) {
			limit -= to_subtract;
			return *this;
		}
	};
}
