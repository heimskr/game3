#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/InteractionSet.h"
#include "game/Inventory.h"
#include "item/Flower.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"

namespace Game3 {
	bool StandardInteractions::interact(const Place &place, Modifiers modifiers) const {
		const Position position = place.position;
		Player &player = *place.player;
		Realm &realm = *place.realm;
		Game &game = realm.getGame();
		Inventory &inventory = *player.getInventory();

		Tileset &tileset = realm.getTileset();
		const auto terrain_tile   = place.getName(Layer::Terrain);
		const auto submerged_tile = place.getName(Layer::Submerged);

		if (!terrain_tile || !submerged_tile)
			return false;

		if (auto *active = inventory.getActive()) {
			if (active->item->canUseOnWorld() && active->item->use(inventory.activeSlot, *active, place, modifiers, {0.f, 0.f}))
				return true;

			if (active->hasAttribute("base:attribute/shovel"_id)) {
				if (*submerged_tile == "base:tile/ash"_id) {
					realm.setTile(Layer::Submerged, position, "base:tile/empty"_id);
					player.give({game, "base:item/ash"_id, 1});
					realm.reupload();
					return true;
				}
			}
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

		if (item && attribute && !player.hasTooldown()) {
			if (auto *stack = inventory.getActive()) {
				if (stack->hasAttribute(*attribute) && !inventory.add({game, *item, 1})) {
					player.setTooldown(1.f);
					if (stack->reduceDurability())
						inventory.erase(inventory.activeSlot);
					// setTooldown doesn't call notifyOwner on the player's inventory, so we have to do it here.
					inventory.notifyOwner();
					return true;
				}
			}
		}

		if (tileset.isInCategory(*submerged_tile, "base:category/plantable"_id)) {
			if (auto iter = game.itemsByAttribute.find("base:attribute/plantable"_id); iter != game.itemsByAttribute.end()) {
				for (const auto &item: iter->second) {
					if (auto cast = std::dynamic_pointer_cast<Flower>(item); cast && cast->tilename == *submerged_tile) {
						player.give({game, item});
						realm.setTile(Layer::Submerged, position, tileset.getEmptyID());
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
			if (std::uniform_real_distribution(0., 1.)(threadContext.rng) < M_PI / 10.)
				place.set(Layer::Objects, "base:tile/charred_stump");
			else
				ItemStack(place.getGame(), "base:item/wood").spawn(place.realm, place.position);

			place.set(Layer::Submerged, "base:tile/ash");
		}

		if (place.getName(Layer::Objects) == "base:tile/charred_stump"_id) {
			place.realm->setTile(Layer::Objects, place.position, "base:tile/empty"_id);
			return true;
		}

		return false;
	}
}
