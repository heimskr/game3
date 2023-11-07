#include "types/TileUpdateContext.h"

namespace Game3 {
	TileUpdateContext::TileUpdateContext():
		limit(2) {}

	TileUpdateContext::TileUpdateContext(uint32_t limit_):
		limit(limit_) {}
}
