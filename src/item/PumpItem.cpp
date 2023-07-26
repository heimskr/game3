#include <iostream>

#include "Log.h"
#include "Position.h"
#include "Tileset.h"
#include "entity/Player.h"
#include "biome/Biome.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/PumpItem.h"
#include "realm/Cave.h"
#include "threading/ThreadContext.h"
#include "tileentity/Pump.h"
#include "tileentity/Ghost.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "worldgen/CaveGen.h"

namespace Game3 {
	bool PumpItem::use(Slot slot, ItemStack &stack, const Place &place, Modifiers modifiers, std::pair<float, float>) {
		Realm &realm = *place.realm;
		Game  &game  = realm.getGame();
		assert(game.getSide() == Side::Server);

		const PlayerPtr &player   = place.player;
		const Position  &position = place.position;

		auto pump = std::dynamic_pointer_cast<Pump>(realm.tileEntityAt(position));

		if (modifiers.onlyShift()) {
			if (!pump)
				return false;

			realm.removeSafe(pump);
			player->inventory->add(stack.withCount(1));
			player->inventory->notifyOwner();
			return true;
		}

		if (!realm.isPathable(position) || realm.hasTileEntityAt(position)) {
			INFO("Pathable: " << realm.isPathable(position) << ", has TE: " << realm.hasTileEntityAt(position));
			return false;
		}

		std::shared_ptr<Pump> tile_entity = TileEntity::create<Pump>(game, position);

		if (realm.add(tile_entity) != nullptr) {
			game.toServer().tileEntitySpawned(tile_entity);
			if (--stack.count == 0)
				player->inventory->erase(slot);
			player->inventory->notifyOwner();
			return true;
		}

		INFO("Oops, couldn't add...?");
		return false;
	}
}
