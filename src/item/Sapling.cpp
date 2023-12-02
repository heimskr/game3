#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/Sapling.h"
#include "realm/Realm.h"
#include "util/Util.h"

namespace Game3 {
	bool Sapling::use(Slot slot, ItemStack &stack, const Place &place, Modifiers, std::pair<float, float>, Hand) {
		Player &player = *place.player;
		Realm &realm = *place.realm;
		assert(realm.getSide() == Side::Server);

		Tileset &tileset = realm.getTileset();

		if (auto tilename = place.getName(Layer::Terrain); !tilename || !tileset.isInCategory(*tilename, getSoilCategory()))
			return false;

		if (!place.isPathable() || place.get(Layer::Submerged) != 0)
			return false;

		const InventoryPtr inventory = player.getInventory(0);
		auto inventory_lock = inventory->uniqueLock();
		return plant(inventory, slot, stack, place);
	}

	bool Sapling::drag(Slot slot, ItemStack &stack, const Place &place, Modifiers modifiers) {
		return use(slot, stack, place, modifiers, {0.f, 0.f}, Hand::None);
	}

	bool Sapling::plant(InventoryPtr inventory, Slot slot, ItemStack &stack, const Place &place) {
		if (stack.count == 0) {
			inventory->erase(slot);
			inventory->notifyOwner();
			return false;
		}

		place.set(Layer::Submerged, choose(getTreeTypes()));
		{
			std::unique_lock<std::shared_mutex> lock;
			place.realm->tileProvider.findPathState(place.position, &lock) = 0;
		}

		if (--stack.count == 0)
			inventory->erase(slot);
		inventory->notifyOwner();
		return true;
	}
}
