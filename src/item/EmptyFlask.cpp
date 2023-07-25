#include "Position.h"
#include "Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/EmptyFlask.h"
#include "realm/Realm.h"

namespace Game3 {
	bool EmptyFlask::use(Slot slot, ItemStack &stack, const Place &place, Modifiers, std::pair<float, float>) {
		auto &player = *place.player;
		auto &realm  = *place.realm;
		assert(realm.getSide() == Side::Server);

		if (std::optional<FluidTile> tile = realm.tryFluid(place.position); tile && FluidTile::FULL <= tile->level) {
			auto fluid = realm.getGame().getFluid(tile->id);
			if (!fluid || !fluid->flaskName)
				return false;

			if (tile->level != FluidTile::INFINITE) {
				tile->level = 0;
				realm.setFluid(place.position, *tile);
			}

			{
				auto lock = player.inventory->uniqueLock();
				if (--stack.count == 0)
					player.inventory->erase(slot);
			}

			player.give(ItemStack(realm.getGame(), fluid->flaskName, 1), slot);
			player.inventory->notifyOwner();
			return true;
		}

		return false;
	}
}
