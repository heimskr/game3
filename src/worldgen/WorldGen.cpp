#include "worldgen/WorldGen.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	namespace WorldGen {
		ThreadPool pool{5};
	}

	double getDefaultWetness() {
		return -.05;
	}

	void from_json(const nlohmann::json &json, WorldGenParams &params) {
		params.wetness = json.at("wetness");
		params.stoneLevel = json.at("stoneLevel");
		params.forestThreshold = json.at("forestThreshold");
		params.antiforestThreshold = json.at("antiforestThreshold");
		params.biomeZoom = json.at("biomeZoom");
	}

	void to_json(nlohmann::json &json, const WorldGenParams &params) {
		json["wetness"] = params.wetness;
		json["stoneLevel"] = params.stoneLevel;
		json["forestThreshold"] = params.forestThreshold;
		json["antiforestThreshold"] = params.antiforestThreshold;
		json["biomeZoom"] = params.biomeZoom;
	}
}
