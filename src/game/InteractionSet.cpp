#include "entity/Player.h"
#include "game/Game.h"
#include "game/InteractionSet.h"
#include "game/Inventory.h"
#include "graphics/Tileset.h"
#include "item/Flower.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "tile/Tile.h"
#include "types/Position.h"

namespace Game3 {
	bool StandardInteractions::interact(const Place &place, Modifiers modifiers, const ItemStackPtr &used_item, Hand hand) const {
		const Position position = place.position;
		PlayerPtr player = place.player;
		RealmPtr realm = place.realm;
		GamePtr game = realm->getGame();
		InventoryPtr inventory = player->getInventory(0);

		Tileset &tileset = realm->getTileset();
		const auto submerged_tile = place.getName(Layer::Submerged);

		if (!submerged_tile) {
			return false;
		}

		auto inventory_lock = inventory->uniqueLock();

		auto get_active = [&] {
			return used_item? used_item : inventory->getActive();
		};

		if (!used_item) {
			if (ItemStackPtr active = get_active()) {
				if (active->item->canUseOnWorld() && active->item->use(inventory->activeSlot, active, place, modifiers, {0.f, 0.f})) {
					return true;
				}
			}
		} else if (used_item->item->use(player->getHeldSlot(hand), used_item, place, modifiers, hand)) {
			return true;
		}

		if (tileset.isInCategory(*submerged_tile, "base:category/plantable"_id)) {
			if (auto iter = game->itemsByAttribute.find("base:attribute/plantable"_id); iter != game->itemsByAttribute.end()) {
				for (const ItemPtr &item: iter->second) {
					if (auto cast = std::dynamic_pointer_cast<Flower>(item); cast && cast->tilename == *submerged_tile) {
						player->give(ItemStack::create(game, item));
						realm->setTile(Layer::Submerged, position, tileset.getEmptyID());
						return true;
					}
				}
			}
		}

		return false;
	}

	bool StandardInteractions::damageGround(const Place &place) const {
		if (place.getName(Layer::Objects) == "base:tile/charred_stump"_id) {
			place.realm->setTile(Layer::Objects, place.position, "base:tile/empty"_id);
			ItemStack::spawn(place, place.getGame(), "base:item/wood");
			return true;
		}

		bool result = false;

		for (Layer layer: allLayers) {
			result = place.getTile(layer)->damage(place, layer) || result;
		}

		return result;
	}
}
