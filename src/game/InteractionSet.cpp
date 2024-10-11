#include "entity/Player.h"
#include "game/Game.h"
#include "game/InteractionSet.h"
#include "game/Inventory.h"
#include "graphics/Tileset.h"
#include "item/Flower.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "types/Position.h"

namespace Game3 {
	bool StandardInteractions::interact(const Place &place, Modifiers modifiers, const ItemStackPtr &used_item, Hand hand) const {
		const Position position = place.position;
		PlayerPtr player = place.player;
		RealmPtr realm = place.realm;
		GamePtr game = realm->getGame();
		InventoryPtr inventory = player->getInventory(0);

		Tileset &tileset = realm->getTileset();
		const auto terrain_tile   = place.getName(Layer::Terrain);
		const auto submerged_tile = place.getName(Layer::Submerged);

		if (!terrain_tile || !submerged_tile) {
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

				if (active->hasAttribute("base:attribute/shovel"_id)) {
					if (*submerged_tile == "base:tile/ash"_id) {
						realm->setTile(Layer::Submerged, position, "base:tile/empty"_id);
						player->give(ItemStack::create(game, "base:item/ash"_id, 1));
						realm->reupload();
						return true;
					}
				}
			}
		} else if (used_item->item->use(player->getHeldSlot(hand), used_item, place, modifiers, hand)) {
			return true;
		}

		std::optional<Identifier> item;
		std::optional<Identifier> attribute;

		if (*terrain_tile == "base:tile/sand"_id) {
			item.emplace("base:item/sand"_id);
			attribute.emplace("base:attribute/shovel"_id);
		} else if (*terrain_tile == "base:tile/shallow_water"_id) {
			item.emplace("base:item/clay"_id);
			attribute.emplace("base:attribute/shovel"_id);
		} else if (*terrain_tile == "base:tile/volcanic_sand"_id) {
			item.emplace("base:item/volcanic_sand"_id);
			attribute.emplace("base:attribute/shovel"_id);
		} else if (tileset.isInCategory(*terrain_tile, "base:category/dirt")) {
			item.emplace("base:item/dirt"_id);
			attribute.emplace("base:attribute/shovel"_id);
		}

		if (used_item && item && attribute && !player->hasTooldown()) {
			if (used_item->hasAttribute(*attribute) && !inventory->add(ItemStack::create(game, *item, 1))) {
				player->setTooldown(1.f);
				if (used_item->reduceDurability()) {
					inventory->erase(player->getHeldSlot(hand));
				}
				// setTooldown doesn't call notifyOwner on the player's inventory, so we have to do it here.
				inventory->notifyOwner({});
				return true;
			}
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
		const Tileset &tileset = place.realm->getTileset();

		if (const auto submerged = place.get(Layer::Submerged); submerged && tileset.isInCategory(*submerged, "base:category/trees")) {
			if (threadContext.random(0.0, 1.0) < M_PI / 10.) {
				place.set(Layer::Objects, "base:tile/charred_stump");
			} else {
				ItemStack::spawn(place, place.getGame(), "base:item/wood");
			}

			place.set(Layer::Submerged, "base:tile/ash");
		}

		if (place.getName(Layer::Objects) == "base:tile/charred_stump"_id) {
			place.realm->setTile(Layer::Objects, place.position, "base:tile/empty"_id);
			return true;
		}

		return false;
	}
}
