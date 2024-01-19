#include "data/Richness.h"
#include "game/Game.h"
#include "game/Resource.h"

namespace Game3 {
	Richness Richness::getRandom(const Game &game) {
		Richness out;

		for (const auto &[identifier, resource]: game.registry<ResourceRegistry>())
			out.richnesses[identifier] = resource->sampleRichness();

		return out;
	}
}
