#include <iostream>

#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "biome/Biome.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/CaveEntrance.h"
#include "realm/Cave.h"
#include "threading/ThreadContext.h"
#include "tileentity/Building.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "worldgen/CaveGen.h"

namespace Game3 {
	bool CaveEntrance::use(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers, std::pair<float, float>) {
		Realm &realm = *place.realm;
		GamePtr game = realm.getGame();
		assert(game->getSide() == Side::Server);

		const PlayerPtr &player   = place.player;
		const Position  &position = place.position;

		const Position exit_position = position + Position(1, 0);

		if (!realm.isPathable(exit_position) || !realm.isPathable(position) || realm.hasTileEntityAt(position))
			return false;

		std::optional<RealmID> realm_id;
		Position entrance;

		for (const auto &[index, tile_entity]: realm.tileEntities) {
			if (tile_entity->tileID == "base:tile/cave"_id && tile_entity->is("base:te/building"_id)) {
				if (auto building = std::dynamic_pointer_cast<Building>(tile_entity)) {
					realm_id = building->innerRealmID;
					if (auto cave_realm = std::dynamic_pointer_cast<Cave>(game->getRealm(*realm_id)))
						++cave_realm->entranceCount;
					else
						throw std::runtime_error("Cave entrance leads to realm " + std::to_string(*realm_id) + ", which isn't a cave");
					entrance = building->entrance;
					break;
				}
			}
		}

		bool emplaced = false;

		if (!realm_id) {
			realm_id = game->newRealmID();
			const int cave_seed = -2 * realm.seed - 5 + game->cavesGenerated;
			auto new_realm = Realm::create<Cave>(game, *realm_id, realm.id, cave_seed);
			new_realm->outdoors = false;
			Position entrance_position;
			WorldGen::generateCaveFull(new_realm, threadContext.rng, cave_seed, exit_position, entrance_position, realm.id, {{-1, -1}, {1, 1}});
			entrance = entrance_position;
			game->addRealm(*realm_id, new_realm);
			++game->cavesGenerated;
			emplaced = true;
		}

		if (auto tile_entity = TileEntity::spawn<Building>(place.realm, "base:tile/cave"_id, position, *realm_id, entrance)) {
			game->toServer().tileEntitySpawned(tile_entity);
			InventoryPtr inventory = player->getInventory(0);
			assert(inventory);
			inventory->decrease(stack, slot, 1, true);
			return true;
		}

		if (emplaced)
			game->removeRealm(*realm_id);

		return false;
	}
}
