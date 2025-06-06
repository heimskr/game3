#include "util/Log.h"
#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "item/TerrainSeed.h"
#include "realm/Realm.h"

namespace Game3 {
	TerrainSeed::TerrainSeed(ItemID id, std::string name, Layer targetLayer, Identifier targetTilename, Layer replacementLayer, Identifier replacementTilename, MoneyCount basePrice, ItemCount maxCount):
		Item(id, std::move(name), basePrice, maxCount),
		targetTilename(std::move(targetTilename)),
		replacementTilename(std::move(replacementTilename)),
		targetLayer(targetLayer),
		replacementLayer(replacementLayer) {}

	bool TerrainSeed::use(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers, std::pair<float, float>) {
		Realm &realm = *place.realm;
		Tileset &tileset = realm.getTileset();

		std::optional<TileID> tile = realm.tryTile(targetLayer, place.position);
		if (!tile) {
			return false;
		}

		if (replacementLayer != targetLayer && place.get(replacementLayer)) {
			return false;
		}

		bool match{};

		if (targetTilename.getPathStart() == "category") {
			match = tileset.isInCategory(*tile, targetTilename);
		} else {
			match = tileset[*tile] == targetTilename;
		}

		if (match) {
			place.player->getInventory(0)->decrease(stack, slot, 1, true);
			place.set(replacementLayer, replacementTilename);
			return true;
		}

		return false;
	}

	bool TerrainSeed::drag(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets, DragAction) {
		return use(slot, stack, place, modifiers, offsets);
	}
}
