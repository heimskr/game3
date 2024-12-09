#include "Log.h"
#include "types/Position.h"
#include "realm/Cave.h"
#include "tile/CaveTile.h"

#include <boost/json.hpp>

namespace Game3 {
	CaveTile::CaveTile(Identifier identifier_, ItemStackPtr stack_, Identifier floor_):
		MineableTile(std::move(identifier_), std::move(stack_), true),
		floor(std::move(floor_)) {}

	bool CaveTile::interact(const Place &place, Layer layer, const ItemStackPtr &used_item, Hand hand) {
		if (!MineableTile::interact(place, layer, used_item, hand)) {
			return false;
		}

		if (auto terrain = place.get(Layer::Terrain); !terrain || *terrain == 0) {
			place.set(Layer::Terrain, floor);
		}

		if (auto cave = std::dynamic_pointer_cast<Cave>(place.realm)) {
			cave->reveal(place.position, true);
		}

		return true;
	}
}
