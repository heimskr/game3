#include "Position.h"
#include "Tileset.h"
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
		if (realm.type != "base:realm/overworld"_id)
			return false;

		auto &tileset = realm.getTileset();

		if (auto tilename = place.getName(Layer::Terrain); !tilename || !tileset.isInCategory(*tilename, "base:category/plant_soil"_id))
			return false;

		static const std::array<Identifier, 3> trees {"base:tile/tree1"_id, "base:tile/tree2"_id, "base:tile/tree3"_id};
		auto new_tree = TileEntity::create<Tree>(realm.getGame(), choose(trees), "base:tile/tree0"_id, place.position, 0.f);
		if (place.isPathable() && realm.add(new_tree)) {
			realm.getGame().toServer().tileEntitySpawned(new_tree);
			if (--stack.count == 0)
				player.inventory->erase(slot);
			player.inventory->notifyOwner();
			realm.tileProvider.findPathState(place.position) = 0;
			return true;
		}

		return false;
	}
}
