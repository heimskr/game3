#include "lib/JSON.h"
#include "types/VillageOptions.h"

namespace Game3 {
	VillageOptions::VillageOptions(int width_, int height_, int padding_):
		width(width_), height(height_), padding(padding_) {}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const VillageOptions &village_options) {
		auto &object = json.emplace_object();
		object["width"]   = village_options.width;
		object["height"]  = village_options.height;
		object["padding"] = village_options.padding;
	}

	VillageOptions tag_invoke(boost::json::value_to_tag<VillageOptions>, const boost::json::value &json) {
		int width   = getNumber<int>(json.at("width"));
		int height  = getNumber<int>(json.at("height"));
		int padding = getNumber<int>(json.at("padding"));
		return {width, height, padding};
	}
}
