#include "Log.h"
#include "types/Position.h"
#include "realm/Cave.h"
#include "tile/CaveTile.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	CaveTile::CaveTile(Identifier identifier_, ItemStack stack_, Identifier floor_):
		MineableTile(std::move(identifier_), std::move(stack_), true),
		floor(std::move(floor_)) {}

	bool CaveTile::interact(const Place &place, Layer layer) {
		if (!MineableTile::interact(place, layer))
			return false;

		if (auto terrain = place.get(Layer::Terrain); !terrain || *terrain == 0)
			place.set(Layer::Terrain, floor);

		if (auto cave = std::dynamic_pointer_cast<Cave>(place.realm))
			cave->reveal(place.position, true);

		return true;
	}
}
