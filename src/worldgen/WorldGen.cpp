#include "worldgen/WorldGen.h"

#include <boost/json.hpp>

namespace Game3 {
	namespace WorldGen {
		ThreadPool pool{5};
	}

	double getDefaultWetness() {
		return -.05;
	}

	void from_json(const boost::json::value &json, WorldGenParams &params) {
		params.wetness = json.at("wetness");
		params.stoneLevel = json.at("stoneLevel");
		params.forestThreshold = json.at("forestThreshold");
		params.antiforestThreshold = json.at("antiforestThreshold");
		params.biomeZoom = json.at("biomeZoom");
	}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const WorldGenParams &params) {
		json["wetness"] = params.wetness;
		json["stoneLevel"] = params.stoneLevel;
		json["forestThreshold"] = params.forestThreshold;
		json["antiforestThreshold"] = params.antiforestThreshold;
		json["biomeZoom"] = params.biomeZoom;
	}
}
