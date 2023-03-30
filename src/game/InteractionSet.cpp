#include "Position.h"
#include "Tiles.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/InteractionSet.h"
#include "game/Inventory.h"
#include "realm/Realm.h"

namespace Game3 {
	bool StandardInteractions::interact(const Place &place) const {
		const auto &position = place.position;
		auto &player = *place.player;
		auto &realm  = *place.realm;
		auto &inventory = *player.inventory;

		const Index index = realm.getIndex(position);
		auto &tilemap1 = realm.tilemap1;
		auto &tilemap2 = realm.tilemap2;
		const TileID tile1 = place.getLayer1();
		const TileID tile2 = place.getLayer2();

		if (auto *active = inventory.getActive()) {
			if (active->item->canUseOnWorld() && active->item->use(inventory.activeSlot, *active, place))
				return true;

			if (active->has(ItemAttribute::Hammer)) {
				auto &tileset = *tileSets.at(realm.type);
				const TileID tile2 = tilemap2->tiles.at(index);
				ItemStack stack;
				if (tileset.getItemStack(tile2, stack) && !inventory.add(stack)) {
					if (active->reduceDurability())
						inventory.erase(inventory.activeSlot);
					realm.setLayer2(position, tileset.getEmpty());
					return true;
				}
			}

			if (active->has(ItemAttribute::Shovel)) {
				switch (tile2) {
					case Monomap::ASH: {
						realm.setLayer2(position, Monomap::EMPTY);
						player.give({Item::ASH, 1});
						realm.getGame().activateContext();
						realm.reupload();
						return true;
					}

					default:
						return false;
				}
			}
		}

		std::optional<ItemID> item;
		std::optional<ItemAttribute> attribute;

		if (tile1 == Monomap::SAND) {
			item.emplace(Item::SAND);
			attribute.emplace(ItemAttribute::Shovel);
		} else if (tile1 == Monomap::SHALLOW_WATER) {
			item.emplace(Item::CLAY);
			attribute.emplace(ItemAttribute::Shovel);
		} else if (Monomap::dirtSet.contains(tile1)) {
			item.emplace(Item::DIRT);
			attribute.emplace(ItemAttribute::Shovel);
		} else if (tile1 == Monomap::STONE) {
			item.emplace(Item::STONE);
			attribute.emplace(ItemAttribute::Pickaxe);
		}

		if (item && attribute && !player.hasTooldown()) {
			if (auto *stack = inventory.getActive()) {
				if (stack->has(*attribute) && !inventory.add({*item, 1})) {
					player.setTooldown(1.f);
					if (stack->reduceDurability())
						inventory.erase(inventory.activeSlot);
					else
						// setTooldown doesn't call notifyOwner on the player's inventory, so we have to do it here.
						player.inventory->notifyOwner();
					return true;
				}
			}
		}

		return false;
	}
}
