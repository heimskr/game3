#include <iostream>

#include "algorithm/Spiral.h"
#include "biome/Biome.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "graphics/Tileset.h"
#include "item/CaveEntrance.h"
#include "realm/Cave.h"
#include "threading/ThreadContext.h"
#include "tileentity/Building.h"
#include "types/Position.h"
#include "ui/Window.h"
#include "worldgen/CaveGen.h"

namespace {
	constexpr size_t LADDER_ATTEMPTS = 49;
}

namespace Game3 {
	bool CaveEntrance::use(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers, std::pair<float, float>) {
		RealmPtr realm = place.realm;
		GamePtr game = realm->getGame();
		assert(game->getSide() == Side::Server);

		const PlayerPtr &player   = place.player;
		const Position  &position = place.position;

		const Position exit_position = position + Position(1, 0);

		if (!realm->isPathable(exit_position) || !realm->isPathable(position) || realm->hasTileEntityAt(position))
			return false;

		std::optional<RealmID> realm_id;
		Position entrance;

		for (const auto &[index, tile_entity]: realm->tileEntities) {
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

		std::shared_ptr<Cave> new_realm;

		if (realm_id) {
			// Attempt to place a ladder in the existing cave realm.
			auto cave = std::dynamic_pointer_cast<Cave>(game->getRealm(*realm_id));
			if (!cave) {
				WARN("Can't place ladder in cave: cave is null");
			} else {
				const Position position_in_cave = position / Cave::SCALE;
				for (size_t i = 0; i < LADDER_ATTEMPTS; ++i) {
					Position ladder_position = position_in_cave + getSpiralPosition(i);
					if (!cave->isPathable(ladder_position) || cave->hasTileEntityAt(ladder_position))
						continue;
					if (auto ladder = TileEntity::spawn<Building>(cave, "base:tile/ladder", ladder_position, realm->getID(), exit_position)) {
						game->toServer().tileEntitySpawned(ladder);
						entrance = ladder_position + Position(1, 0);
						break;
					}
				}
			}
		} else {
			realm_id = game->newRealmID();
			const int cave_seed = -2 * realm->seed - 5 + game->cavesGenerated;
			new_realm = Realm::create<Cave>(game, *realm_id, realm->id, cave_seed);
			new_realm->outdoors = false;
			Position entrance_position;
			WorldGen::generateCaveFull(new_realm, threadContext.rng, cave_seed, exit_position, entrance_position, realm->id, {{-1, -1}, {1, 1}});
			entrance = entrance_position + Position(1, 0);
			game->addRealm(*realm_id, new_realm);
			++game->cavesGenerated;
		}

		if (auto tile_entity = TileEntity::spawn<Building>(place.realm, "base:tile/cave"_id, position, *realm_id, entrance)) {
			game->toServer().tileEntitySpawned(tile_entity);
			InventoryPtr inventory = player->getInventory(0);
			assert(inventory);
			inventory->decrease(stack, slot, 1, true);
			return true;
		}

		// If we get to this point, we couldn't spawn the cave entrance, so we need to remove the orphaned new cave realm.
		if (new_realm)
			game->removeRealm(new_realm);

		return false;
	}
}
