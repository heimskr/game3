#include "Position.h"
#include "Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "item/Sapling.h"
#include "realm/Realm.h"
#include "tileentity/Tree.h"
#include "util/Util.h"

namespace Game3 {
	bool Sapling::use(Slot slot, ItemStack &stack, const Place &place) {
		auto &player = *place.player;
		auto &realm  = *place.realm;
		if (realm.type != "base:realm/overworld"_id)
			return false;

		const auto index = realm.getIndex(place.position);
		const auto &tilemap = *realm.tilemap1;
		const auto &tileset = *tilemap.tileset;

		if (!realm.tilemap1->tileset->isInCategory(tileset[tilemap[index]], "base:category/tree_soil"_id))
			return false;

		static const std::array<Identifier, 3> trees {"base:tile/tree1"_id, "base:tile/tree2"_id, "base:tile/tree3"_id};
		if (realm.pathMap[index] && nullptr != realm.add(TileEntity::create<Tree>(realm.getGame(), choose(trees), "base:tile/tree0"_id, place.position, 0.f))) {

			if (--stack.count == 0)
				player.inventory->erase(slot);
			player.inventory->notifyOwner();
			realm.pathMap[index] = 0;
			return true;
		}

		return false;
	}
}
