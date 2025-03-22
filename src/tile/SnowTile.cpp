#include "Log.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "tile/SnowTile.h"
#include "types/Position.h"

namespace Game3 {
	SnowTile::SnowTile():
		Tile(ID()) {}

	std::optional<FluidTile> SnowTile::yieldFluid(const Place &place) {
		if (!cachedFluidID) {
			cachedFluidID = place.getGame()->getFluid("base:fluid/powdered_snow")->registryID;
		}

		return FluidTile{*cachedFluidID, 1000};
	}
}
