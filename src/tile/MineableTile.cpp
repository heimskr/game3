#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "lib/JSON.h"
#include "realm/Realm.h"
#include "registry/Registries.h"
#include "tile/MineableTile.h"
#include "types/Position.h"
#include "util/Log.h"

namespace Game3 {
	MineableTile::MineableTile(Identifier identifier, ItemStackPtr stack, bool consumable):
		Tile(std::move(identifier)),
		stack(std::move(stack)),
		consumable(consumable) {}

	bool MineableTile::interact(const Place &place, Layer layer, const ItemStackPtr &held_item, Hand hand) {
		if (layer == Layer::Terrain) {
			// Terrain isn't mineable.
			return false;
		}

		PlayerPtr player = place.player;
		if (!player) {
			return false;
		}

		InventoryPtr inventory = player->getInventory(0);
		if (!inventory) {
			return false;
		}

		if (!held_item || !held_item->hasAttribute("base:attribute/pickaxe")) {
			return false;
		}

		if (consumable) {
			place.set(layer, 0);
		}

		if (held_item->reduceDurability()) {
			inventory->erase(player->getHeldSlot(hand));
		}

		inventory->notifyOwner({});

		player->give(stack);

		return true;
	}

	bool MineableTile::damage(const Place &place, Layer layer) {
		if (layer == Layer::Terrain || !consumable) {
			return false;
		}

		place.set(layer, 0);
		auto entity = place.realm->spawn<ItemEntity>(place.position, stack);
		entity->velocity.z = 10;

		return true;
	}

	void TileRegistry::addMineable(Identifier tilename, const ItemStackPtr &stack, bool consumable) {
		add(tilename, std::make_unique<MineableTile>(tilename, stack->copy(), consumable));
	}
}
