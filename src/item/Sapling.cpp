#include "Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/Sapling.h"
#include "realm/Realm.h"
#include "tileentity/Tree.h"
#include "util/Util.h"

namespace Game3 {
	bool Sapling::use(Slot slot, ItemStack &stack, const Place &place, Modifiers, std::pair<float, float>) {
		auto &player = *place.player;
		auto &realm  = *place.realm;
		assert(realm.getSide() == Side::Server);
		if (realm.type != getRealmType())
			return false;

		Tileset &tileset = realm.getTileset();

		if (auto tilename = place.getName(Layer::Terrain); !tilename || !tileset.isInCategory(*tilename, getSoilCategory()))
			return false;

		if (!place.isPathable() || place.get(Layer::Submerged) != 0)
			return false;

		place.set(Layer::Submerged, choose(getTreeTypes()));
		const InventoryPtr inventory = player.getInventory();
		if (--stack.count == 0)
			inventory->erase(slot);
		inventory->notifyOwner();
		std::unique_lock<std::shared_mutex> lock;
		realm.tileProvider.findPathState(place.position, &lock) = 0;
		return true;
	}

	bool Sapling::drag(Slot slot, ItemStack &stack, const Place &place, Modifiers modifiers) {
		return use(slot, stack, place, modifiers, {0.f, 0.f});
	}
}
