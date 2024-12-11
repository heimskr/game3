#include "lib/JSON.h"
#include "worldgen/WorldGen.h"

namespace Game3 {
	namespace WorldGen {
		ThreadPool pool{5};
	}

	double getDefaultWetness() {
		return -.05;
	}

	WorldGenParams tag_invoke(boost::json::value_to_tag<WorldGenParams>, const boost::json::value &json) {
		WorldGenParams out;
		out.wetness = getDouble(json.at("wetness"));
		out.stoneLevel = getDouble(json.at("stoneLevel"));
		out.forestThreshold = getDouble(json.at("forestThreshold"));
		out.antiforestThreshold = getDouble(json.at("antiforestThreshold"));
		out.biomeZoom = getDouble(json.at("biomeZoom"));
		return out;
	}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const WorldGenParams &params) {
		auto &object = json.emplace_object();
		object["wetness"] = params.wetness;
		object["stoneLevel"] = params.stoneLevel;
		object["forestThreshold"] = params.forestThreshold;
		object["antiforestThreshold"] = params.antiforestThreshold;
		object["biomeZoom"] = params.biomeZoom;
	}
}
