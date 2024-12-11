#include "data/Richness.h"
#include "game/Game.h"
#include "game/Resource.h"
#include "lib/JSON.h"

namespace Game3 {
	std::optional<double> Richness::operator[](const Identifier &identifier) const {
		if (auto iter = richnesses.find(identifier); iter != richnesses.end()) {
			return iter->second;
		}
		return std::nullopt;
	}

	Richness Richness::makeRandom(const Game &game) {
		Richness out;

		for (const auto &[identifier, resource]: game.registry<ResourceRegistry>()) {
			if (resource->sampleLikelihood()) {
				out.richnesses[identifier] = resource->sampleRichness();
			}
		}

		return out;
	}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const Richness &richness) {
		json = boost::json::value_from(richness.richnesses);
	}

	Richness tag_invoke(boost::json::value_to_tag<Richness>, const boost::json::value &json) {
		Richness out;
		out.richnesses = boost::json::value_to<decltype(out.richnesses)>(json);
		return out;
	}
}
