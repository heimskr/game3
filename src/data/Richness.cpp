#include "data/Richness.h"
#include "game/Game.h"
#include "game/Resource.h"

namespace Game3 {
	std::optional<double> Richness::operator[](const Identifier &identifier) {
		if (auto iter = richnesses.find(identifier); iter != richnesses.end())
			return iter->second;
		return std::nullopt;
	}

	Richness Richness::makeRandom(const Game &game) {
		Richness out;

		for (const auto &[identifier, resource]: game.registry<ResourceRegistry>())
			if (resource->sampleLikelihood())
				out.richnesses[identifier] = resource->sampleRichness();

		return out;
	}

	void to_json(nlohmann::json &json, const Richness &richness) {
		json = richness.richnesses;
	}

	void from_json(const nlohmann::json &json, Richness &richness) {
		richness.richnesses = json;
	}
}
