#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/Mead.h"
#include "realm/Realm.h"

namespace Game3 {
	bool Mead::use(Slot slot, ItemStack &stack, const Place &place, Modifiers, std::pair<float, float>) {
		Realm  &realm  = *place.realm;
		assert(realm.getSide() == Side::Server);
		Game   &game   = realm.getGame();
		Player &player = *place.player;

		if (place.position == player.getPosition()) {
			// TODO: implement drinking mead
			return true;
		}

		if (std::optional<FluidTile> tile = realm.tryFluid(place.position); !tile || tile->level == 0) {
			std::shared_ptr<Fluid> fluid = game.registry<FluidRegistry>().at("base:fluid/mead"_id);
			if (!fluid)
				return false;

			realm.setFluid(place.position, FluidTile(fluid->registryID, FluidTile::FULL));

			const InventoryPtr inventory = player.getInventory();

			{
				auto lock = inventory->uniqueLock();
				if (--stack.count == 0)
					inventory->erase(slot);
			}

			inventory->notifyOwner();
			return true;
		}

		return false;
	}
}
