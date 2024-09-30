#include "Log.h"
#include "types/Position.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "registry/Registries.h"
#include "tile/MineableTile.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	MineableTile::MineableTile(Identifier identifier_, ItemStackPtr stack_, bool consumable_):
		Tile(std::move(identifier_)), stack(std::move(stack_)), consumable(consumable_) {}

	bool MineableTile::interact(const Place &place, Layer layer, const ItemStackPtr &held_item, Hand hand) {
		if (layer == Layer::Terrain) {
			// Terrain isn't mineable.
			return false;
		}

		PlayerPtr player = place.player;
		if (!player)
			return false;

		InventoryPtr inventory = player->getInventory(0);
		if (!inventory)
			return false;

		if (!held_item || !held_item->hasAttribute("base:attribute/pickaxe"))
			return false;

		if (consumable)
			place.set(layer, 0);

		if (held_item->reduceDurability())
			inventory->erase(player->getHeldSlot(hand));
		inventory->notifyOwner({});

		player->give(stack);

		return true;
	}

	void TileRegistry::addMineable(Identifier tilename, const ItemStackPtr &stack, bool consumable) {
		add(tilename, std::make_unique<MineableTile>(tilename, stack->copy(), consumable));
	}
}
