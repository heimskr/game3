#include <iostream>

#include "Position.h"
#include "Tiles.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "item/CaveEntrance.h"
#include "realm/Cave.h"
#include "tileentity/Building.h"
#include "tileentity/Ghost.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "worldgen/CaveGen.h"

namespace Game3 {
	bool CaveEntrance::use(Slot slot, ItemStack &stack, const std::shared_ptr<Player> &player, const Position &position) {
		auto &realm = *player->getRealm();
		auto &game = realm.getGame();

		const Position exit = position + Position(1, 0);

		if (!realm.isValid(exit) || !realm.pathMap.at(realm.getIndex(exit)))
			return false;

		if (!realm.pathMap.at(realm.getIndex(position)) || realm.hasTileEntityAt(position))
			return false;

		// This is a horrible, ugly hack to fix a problem where entering the cave would break the sprite renderer and make all sprites invisible forever.
		// Resetting the sprite renderer exactly one time fixes things. I'm not sure what the earliest possible time to reset it is.
		// However, doing it here seems to work.
		static bool hacked = false;
		if (!hacked) {
			game.activateContext();
			game.canvas.spriteRenderer = SpriteRenderer(game.canvas);
			hacked = true;
		}

		std::optional<RealmID> realm_id;
		Index entrance = -1;
		
		for (const auto &[index, tile_entity]: realm.tileEntities)
			if (tile_entity->tileID == Monomap::CAVE && tile_entity->getID() == TileEntity::BUILDING)
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
			const int realm_width  = 100;
			const int realm_height = 100;
			// TODO: perhaps let the player choose the seed
			const int cave_seed = -2 * realm.seed - 5 + game.cavesGenerated;

			auto new_tilemap = std::make_shared<Tilemap>(realm_width, realm_height, 16, Realm::textureMap.at(Realm::CAVE));
			// TODO: make a cave realm that handles updateNeighbors to convert exposed void into stone so that cave walls can be mineable
			auto new_realm = Realm::create<Cave>(*realm_id, Realm::CAVE, new_tilemap, cave_seed);
			new_realm->outdoors = false;
			new_realm->setGame(game);
			Position entrance_position;
			WorldGen::generateCave(new_realm, game.dynamicRNG, cave_seed, realm.getIndex(exit), entrance_position, realm.id);
			entrance = new_realm->getIndex(entrance_position);
			game.realms.emplace(*realm_id, new_realm);
			++game.cavesGenerated;
			emplaced = true;
		}

		if (realm.add(TileEntity::create<Building>(Monomap::CAVE, position, *realm_id, entrance)) != nullptr) {
			if (--stack.count == 0)
				player->inventory->erase(slot);
			player->inventory->notifyOwner();
			return true;
		} else if (emplaced)
			game.realms.erase(*realm_id);

		return false;
	}
}
