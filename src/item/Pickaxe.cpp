#include "types/Direction.h"
#include "util/Log.h"
#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "item/Pickaxe.h"
#include "realm/Realm.h"

namespace Game3 {
	Pickaxe::Pickaxe(ItemID id_, std::string name_, MoneyCount base_price, float base_cooldown, Durability max_durability):
		Tool(id_, std::move(name_), base_price, base_cooldown, max_durability, "base:attribute/pickaxe"_id) {}

	bool Pickaxe::use(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers, std::pair<float, float>) {
		const RealmPtr realm = place.realm;
		const Tileset &tileset = realm->getTileset();

		if (auto tile = realm->tryTile(Layer::Soil, place.position); tile && tileset.isInCategory(*tile, "base:category/farmable")) {
			{
				InventoryPtr inventory = place.player->getInventory(0);
				auto lock = inventory->uniqueLock();
				if (stack->reduceDurability()) {
					inventory->erase(slot);
				}
				inventory->notifyOwner({});
			}
			place.set(Layer::Soil, findDirtTilename(place));
			return true;
		}

		return false;
	}

	bool Pickaxe::drag(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets, DragAction) {
		return use(slot, stack, place, modifiers, offsets);
	}

	Identifier Pickaxe::findDirtTilename(const Place &place) {
		const RealmPtr realm = place.realm;
		const Tileset &tileset = realm->getTileset();

		for (const Direction direction: ALL_DIRECTIONS) {
			if (auto tilename = (place + direction).getName(Layer::Soil); tilename && tileset.isInCategory(*tilename, "base:category/tillable")) {
				return *tilename;
			}
		}

		return "base:tile/dirt";
	}
}
