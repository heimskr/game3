#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/Food.h"
#include "realm/Realm.h"

namespace Game3 {
	bool Food::use(Slot slot, ItemStack &stack, const PlayerPtr &player, Modifiers) {
		assert(player->getSide() == Side::Server);

		if (player->heal(getHealedPoints(player)) || getConsumptionForced())
			player->getInventory(0)->decrease(stack, slot, 1, true);

		return true;
	}
}
