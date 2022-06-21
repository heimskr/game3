#include <iostream>

#include "Position.h"
#include "Tiles.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "item/CaveEntrance.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"
#include "worldgen/Cave.h"

namespace Game3 {
	bool CaveEntrance::use(Slot slot, ItemStack &stack, const std::shared_ptr<Player> &player, const Position &position) {
		auto &realm = *player->getRealm();
		auto &game = realm.getGame();

		const Position exit = position + Position(1, 0);

		if (!realm.isValid(exit) || !realm.pathMap.at(realm.getIndex(exit)))
			return false;

		if (!realm.pathMap.at(realm.getIndex(position)) || realm.hasTileEntityAt(position))
			return false;

		std::optional<RealmID> realm_id;
		Index entrance = -1;
		
		for (const auto &[index, tile_entity]: realm.tileEntities)
			if (tile_entity->tileID == Monomap::CAVE && tile_entity->getID() == TileEntity::BUILDING)
				if (auto building = std::dynamic_pointer_cast<Building>(tile_entity)) {
					realm_id = building->innerRealmID;
					entrance = building->entrance;
					break;
				}

		bool emplaced = false;

		if (!realm_id) {
			realm_id = game.newRealmID();
			const int realm_width  = 100;
			const int realm_height = 100;
			// TODO: perhaps let the player choose the seed
			const int cave_seed = -2 * game.activeRealm->seed - 5;

			auto new_tilemap = std::make_shared<Tilemap>(realm_width, realm_height, 16, Realm::textureMap.at(Realm::CAVE));
			// TODO: make a cave realm that handles updateNeighbors to convert exposed void into stone so that cave walls can be mineable
			auto new_realm = Realm::create(*realm_id, Realm::CAVE, new_tilemap, cave_seed);
			new_realm->outdoors = false;
			new_realm->setGame(game);
			Position entrance_position;
			WorldGen::generateCave(new_realm, game.dynamicRNG, cave_seed, exit, entrance_position);
			entrance = new_realm->getIndex(entrance_position);
			game.realms.emplace(*realm_id, new_realm);
			emplaced = true;
		}

		if (nullptr != realm.add(TileEntity::create<Building>(Monomap::MISSING, position, *realm_id, entrance))) {
			if (--stack.count == 0)
				player->inventory->erase(slot);
			player->inventory->notifyOwner();
			return true;
		} else if (emplaced)
			game.realms.erase(*realm_id);

		return false;
	}
}
