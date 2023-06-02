#include "Position.h"
#include "Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "item/Pickaxe.h"
#include "realm/Realm.h"

namespace Game3 {
	bool Pickaxe::use(Slot slot, ItemStack &stack, const Place &place, Modifiers) {
		auto &player = *place.player;

		if (player.hasTooldown())
			return false;

		auto &inventory = *player.inventory;
		auto terrain_tile = place.getName(Layer::Terrain);

		if (!terrain_tile)
			return false;

		std::optional<Identifier> item;

		if (terrain_tile == "base:tile/stone"_id) {
			item = "base:item/stone"_id;
		}

		if (item && !inventory.add({place.getGame(), *item, 1})) {
			player.setTooldown(1.f);
			if (stack.reduceDurability())
				inventory.erase(slot);
			else
				// setTooldown doesn't call notifyOwner on the player's inventory, so we have to do it here.
				inventory.notifyOwner();
			return true;
		}

		return false;
	}
}
