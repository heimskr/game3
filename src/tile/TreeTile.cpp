#include "types/Position.h"
#include "entity/Player.h"
#include "game/Crop.h"
#include "game/Inventory.h"
#include "threading/ThreadContext.h"
#include "tile/TreeTile.h"

namespace Game3 {
	TreeTile::TreeTile(std::shared_ptr<Crop> crop):
		CropTile(ID(), std::move(crop)) {}

	bool TreeTile::interact(const Place &place, Layer layer, const ItemStackPtr &used_item, Hand hand) {
		assert(!crop->stages.empty());

		PlayerPtr player = place.player;
		std::shared_ptr<Game> game = player->getGame();

		const InventoryPtr inventory = player->getInventory(0);
		auto inventory_lock = inventory->uniqueLock();

		if (ItemStackPtr active_stack = inventory->getActive()) {
			if (active_stack->hasAttribute("base:attribute/axe")) {
				auto tilename = place.getName(layer);

				// Remove tree
				place.set(layer, 0);

				// Handle axe durability
				if (active_stack->reduceDurability()) {
					inventory->erase(inventory->activeSlot);
				}

				// Make sure tree is fully grown before giving any products
				if (tilename && isRipe(*tilename)) {
					for (const ItemStackPtr &stack: crop->products.getStacks()) {
						if (ItemStackPtr leftover = inventory->add(stack)) {
							leftover->spawn(Place{place.position, place.realm});
						}
					}
				}

				inventory->notifyOwner({});
				return true;
			}
		}

		return doPartialHarvest(place, layer) || Tile::interact(place, layer, used_item, hand);
	}

	bool TreeTile::damage(const Place &place, Layer layer) {
		if (layer != Layer::Submerged) {
			return CropTile::damage(place, layer);
		}

		if (threadContext.random(0.0, 1.0) < 3.14159265358979323846 / 10.) {
			place.set(Layer::Submerged, "base:tile/charred_stump");
		} else {
			ItemStack::spawn(place, place.getGame(), "base:item/wood");
		}

		place.set(Layer::Submerged, "base:tile/ash");
		return true;
	}
}
