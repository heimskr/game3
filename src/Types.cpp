#include "Types.h"

namespace Game3 {
	const std::vector<Layer> allLayers {Layer::Terrain, Layer::Submerged, Layer::Objects, Layer::Highest};

	Index operator""_idx(unsigned long long value) {
		return static_cast<Index>(value);
	}
}
