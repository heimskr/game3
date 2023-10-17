#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/VoidFlask.h"
#include "realm/Realm.h"

namespace Game3 {
	bool VoidFlask::use(Slot, ItemStack &, const Place &place, Modifiers, std::pair<float, float>) {
		auto &realm  = *place.realm;
		assert(realm.getSide() == Side::Server);

		if (std::optional<FluidTile> tile = realm.tryFluid(place.position); tile && 0 < tile->level) {
			tile->level = 0;
			realm.setFluid(place.position, *tile);
			return true;
		}

		return false;
	}
}
