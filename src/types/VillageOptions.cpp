#include "types/VillageOptions.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	VillageOptions::VillageOptions(int width_, int height_, int padding_):
		width(width_), height(height_), padding(padding_) {}

	void to_json(nlohmann::json &json, const VillageOptions &village_options) {
		json["width"]   = village_options.width;
		json["height"]  = village_options.height;
		json["padding"] = village_options.padding;
	}

	void from_json(const nlohmann::json &json, VillageOptions &village_options) {
		village_options.width   = json.at("width");
		village_options.height  = json.at("height");
		village_options.padding = json.at("padding");
	}
}
