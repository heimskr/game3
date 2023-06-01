#include <iostream>

#include "Position.h"
#include "threading/ThreadContext.h"
#include "Tileset.h"
#include "entity/Player.h"
#include "biome/Biome.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/CaveEntrance.h"
#include "realm/Cave.h"
#include "tileentity/Building.h"
#include "tileentity/Ghost.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "worldgen/CaveGen.h"

namespace Game3 {
	bool CaveEntrance::use(Slot slot, ItemStack &stack, const Place &place, Modifiers) {
		auto &realm = *place.realm;
		auto &game  = realm.getGame();
		assert(game.getSide() == Side::Server);

		const auto &player   = place.player;
		const auto &position = place.position;

		const Position exit_position = position + Position(1, 0);

		if (!realm.isPathable(exit_position) || !realm.isPathable(position) || realm.hasTileEntityAt(position))
			return false;

		std::optional<RealmID> realm_id;
		Position entrance;

		for (const auto &[index, tile_entity]: realm.tileEntities)
			if (tile_entity->tileID == "base:tile/cave"_id && tile_entity->is("base:te/building"_id))
				if (auto building = std::dynamic_pointer_cast<Building>(tile_entity)) {
					realm_id = building->innerRealmID;
					if (auto cave_realm = std::dynamic_pointer_cast<Cave>(game.realms.at(*realm_id)))
						++cave_realm->entranceCount;
					else
						throw std::runtime_error("Cave entrance leads to realm " + std::to_string(*realm_id) + ", which isn't a cave");
					entrance = building->entrance;
					break;
				}

		bool emplaced = false;

		if (!realm_id) {
			realm_id = game.newRealmID();
			const int cave_seed = -2 * realm.seed - 5 + game.cavesGenerated;
			auto new_realm = Realm::create<Cave>(game, *realm_id, realm.id, cave_seed);
			new_realm->outdoors = false;
			Position entrance_position;
			WorldGen::generateCaveFull(new_realm, threadContext.rng, cave_seed, exit_position, entrance_position, realm.id, {{-1, -1}, {1, 1}});
			entrance = entrance_position;
			game.realms.emplace(*realm_id, new_realm);
			++game.cavesGenerated;
			emplaced = true;
		}

		auto tile_entity = TileEntity::create<Building>(game, "base:tile/cave"_id, position, *realm_id, entrance);

		if (realm.add(tile_entity) != nullptr) {
			game.toServer().tileEntitySpawned(tile_entity);
			if (--stack.count == 0)
				player->inventory->erase(slot);
			player->inventory->notifyOwner();
			return true;
		}

		if (emplaced)
			game.realms.erase(*realm_id);

		return false;
	}
}
