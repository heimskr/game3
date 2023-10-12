#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace Game3 {
	/** Doesn't include the fluid layer between Submerged and Objects. */
	enum class Layer: uint8_t {Invalid = 0, Terrain, Submerged, Objects, Highest};

	constexpr auto LAYER_COUNT = static_cast<size_t>(Layer::Highest);
	constexpr std::array<Layer, 4> mainLayers     {Layer::Terrain, Layer::Submerged, Layer::Objects, Layer::Highest};
	constexpr std::array<Layer, 3> collidingLayers{                Layer::Submerged, Layer::Objects, Layer::Highest};
	constexpr std::array<Layer, 4> allLayers      {Layer::Terrain, Layer::Submerged, Layer::Objects, Layer::Highest};

	/** Zero-based. */
	inline size_t getIndex(Layer layer) {
		return static_cast<size_t>(layer) - 1;
	}

	/** One-based by default. */
	inline Layer getLayer(size_t index, bool one_based = true) {
		return static_cast<Layer>(index + (one_based? 0 : 1));
	}
}
