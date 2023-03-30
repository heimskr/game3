#include "Position.h"
#include "Tiles.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "item/Sapling.h"
#include "realm/Realm.h"
#include "tileentity/Tree.h"

namespace Game3 {
	bool Sapling::use(Slot slot, ItemStack &stack, const Place &place) {
		auto &player = *place.player;
		auto &realm  = *place.realm;
		const auto index = realm.getIndex(place.position);

		if (realm.type != Realm::OVERWORLD)
			return false;

		switch (realm.tilemap1->tiles[index]) {
			case Monomap::GRASS:
			case Monomap::GRASS_ALT1:
			case Monomap::GRASS_ALT2:
			case Monomap::LIGHT_GRASS:
			case Monomap::DIRT:
			case Monomap::FOREST_FLOOR:
				break;
			default:
				return false;
		}

		auto &rng = realm.getGame().dynamicRNG;

		if (realm.pathMap[index] && nullptr != realm.add(TileEntity::create<Tree>(rng, Monomap::TREE1 + rng() % 3, Monomap::TREE0, place.position, 0.f))) {
			if (--stack.count == 0)
				player.inventory->erase(slot);
			player.inventory->notifyOwner();
			realm.pathMap[index] = 0;
			return true;
		}

		return false;
	}
}
