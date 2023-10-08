#include "Position.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "graphics/Tileset.h"
#include "item/Pickaxe.h"
#include "realm/Realm.h"
#include "tile/Tile.h"

namespace Game3 {
	bool Pickaxe::use(Slot slot, ItemStack &stack, const Place &place, Modifiers, std::pair<float, float>) {
		Player &player = *place.player;

		if (player.hasTooldown())
			return false;

		Inventory &inventory = *player.getInventory();
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
			// setTooldown doesn't call notifyOwner on the player's inventory, so we have to do it here.
			inventory.notifyOwner();
			return true;
		}

		if (std::optional<Identifier> terrain = place.getName(Layer::Terrain))
			return player.getGame().getTile(*terrain)->interact(place, Layer::Terrain);

		return false;
	}
}
