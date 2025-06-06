#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <string>
#include <unordered_map>

namespace Game3 {
	/** Doesn't include the fluid layer between Submerged and Objects. */
	enum class Layer: uint8_t {Invalid = 0, Bedrock, Soil, Vegetation, Flooring, Snow, Submerged, Objects, Highest};

	const extern std::unordered_map<std::string, Layer> layerMap;

	constexpr auto LAYER_COUNT = static_cast<size_t>(Layer::Highest);
	constexpr std::array mainLayers     {Layer::Bedrock, Layer::Soil, Layer::Vegetation, Layer::Flooring, Layer::Snow, Layer::Submerged, Layer::Objects, Layer::Highest};
	constexpr std::array collidingLayers{                Layer::Soil, Layer::Vegetation, Layer::Flooring, Layer::Snow, Layer::Submerged, Layer::Objects, Layer::Highest};
	constexpr std::array allLayers      {Layer::Bedrock, Layer::Soil, Layer::Vegetation, Layer::Flooring, Layer::Snow, Layer::Submerged, Layer::Objects, Layer::Highest};
	constexpr std::array terrainLayers  {Layer::Bedrock, Layer::Soil, Layer::Vegetation, Layer::Flooring, Layer::Snow};

	/** Zero-based. */
	inline size_t getIndex(Layer layer) {
		return static_cast<size_t>(layer) - 1;
	}

	/** One-based by default. */
	inline Layer getLayer(size_t index, bool one_based = true) {
		return static_cast<Layer>(index + (one_based? 0 : 1));
	}

	inline bool isTerrain(Layer layer) {
		return layer == Layer::Bedrock || layer == Layer::Soil || layer == Layer::Vegetation || layer == Layer::Flooring || layer == Layer::Snow;
	}
}

template <>
struct std::formatter<Game3::Layer> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	static const char * getName(Game3::Layer layer) {
		switch (layer) {
			using enum Game3::Layer;
			case Invalid:    return "Invalid";
			case Bedrock:    return "Bedrock";
			case Soil:       return "Soil";
			case Vegetation: return "Vegetation";
			case Flooring:   return "Flooring";
			case Snow:       return "Snow";
			case Submerged:  return "Submerged";
			case Objects:    return "Objects";
			case Highest:    return "Highest";
			default:
				return "?";
		}
	};

	auto format(Game3::Layer layer, auto &ctx) const {
		return std::format_to(ctx.out(), "{}", getName(layer));
	}
};
