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
	bool Sapling::use(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers, std::pair<float, float>) {
		Player &player = *place.player;
		Realm &realm = *place.realm;
		assert(realm.getSide() == Side::Server);

		Tileset &tileset = realm.getTileset();

		if (auto tilename = place.getName(Layer::Soil); !tilename || !tileset.isInCategory(*tilename, getSoilCategory())) {
			return false;
		}

		if (!place.isPathable() || place.get(Layer::Submerged) != 0) {
			return false;
		}

		const InventoryPtr inventory = player.getInventory(0);
		auto inventory_lock = inventory->uniqueLock();
		return plant(inventory, slot, stack, place, Layer::Submerged);
	}

	bool Sapling::drag(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets, DragAction) {
		return use(slot, stack, place, modifiers, offsets);
	}

	bool Sapling::plant(InventoryPtr inventory, Slot slot, const ItemStackPtr &stack, const Place &place, Layer layer) {
		if (stack->count == 0) {
			auto lock = inventory->uniqueLock();
			inventory->erase(slot);
			inventory->notifyOwner({});
			return false;
		}

		place.set(layer, choose(getTreeTypes()));
		place.realm->setPathable(place.position, false);

		inventory->decrease(stack, slot, 1, true);
		return true;
	}
}
