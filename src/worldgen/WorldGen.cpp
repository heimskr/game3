#include "worldgen/WorldGen.h"

#include <boost/json.hpp>

namespace Game3 {
	namespace WorldGen {
		ThreadPool pool{5};
	}

	double getDefaultWetness() {
		return -.05;
	}

	WorldGenParams tag_invoke(boost::json::value_to_tag<WorldGenParams>, const boost::json::value &json) {
		WorldGenParams out;
		out.wetness = json.at("wetness").as_double();
		out.stoneLevel = json.at("stoneLevel").as_double();
		out.forestThreshold = json.at("forestThreshold").as_double();
		out.antiforestThreshold = json.at("antiforestThreshold").as_double();
		out.biomeZoom = json.at("biomeZoom").as_double();
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
