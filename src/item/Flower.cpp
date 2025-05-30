#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "graphics/Tileset.h"
#include "item/Flower.h"
#include "realm/Realm.h"
#include "types/Position.h"

namespace Game3 {
	Flower::Flower(ItemID identifier, std::string name, Identifier tilename, Identifier smallTilename, Identifier validGround, MoneyCount basePrice, ItemCount maxCount):
	Plantable(std::move(identifier), std::move(name), basePrice, maxCount),
	tilename(std::move(tilename)),
	smallTilename(std::move(smallTilename)),
	validGround(std::move(validGround)) {
		attributes.insert("base:attribute/plantable"_id);
		attributes.insert("base:attribute/flower"_id);
	}

	bool Flower::use(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float>) {
		auto &realm = *place.realm;
		const auto &position = place.position;
		const auto &tileset = realm.getTileset();

		if (modifiers.onlyShift()) {
			if (smallTilename && tileset[realm.getTile(Layer::Terrain, position)] == "base:tile/grass") {
				return plantTile(place.player->getInventory(0), slot, stack, place, Layer::Terrain, smallTilename);
			}
		} else if (realm.isPathable(position) && realm.middleEmpty(position)) {
			if (!validGround || tileset.isInCategory(tileset[realm.getTile(Layer::Terrain, position)], validGround)) {
				return plant(place.player->getInventory(0), slot, stack, place, Layer::Submerged);
			}
		}

		return false;
	}

	bool Flower::plant(InventoryPtr inventory, Slot slot, const ItemStackPtr &stack, const Place &place, Layer layer) {
		return plantTile(std::move(inventory), slot, stack, place, layer, tilename);
	}

	bool Flower::plantTile(InventoryPtr inventory, Slot slot, const ItemStackPtr &stack, const Place &place, Layer layer, const Identifier &tile) {
		if (stack->count == 0) {
			auto lock = inventory->uniqueLock();
			inventory->erase(slot);
			inventory->notifyOwner({});
			return false;
		}

		place.set(layer, tile);

		inventory->decrease(stack, slot, 1, true);
		return true;
	}
}
