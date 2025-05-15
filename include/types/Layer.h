#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <format>

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

template <>
struct std::formatter<Game3::Layer> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(Game3::Layer layer, auto &ctx) const {
		const char *name = nullptr;
		switch (layer) {
			using enum Game3::Layer;
			case Invalid: name = "Invalid"; break;
			case Terrain: name = "Terrain"; break;
			case Objects: name = "Objects"; break;
			case Highest: name = "Highest"; break;
			case Submerged: name = "Submerged"; break;
			default: name = "?"; break;
		}
		return std::format_to(ctx.out(), "{}", name);
	}
};
