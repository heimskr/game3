#include "util/Log.h"
#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "item/TerrainSeed.h"
#include "realm/Realm.h"

namespace Game3 {
	TerrainSeed::TerrainSeed(ItemID id_, std::string name_, Identifier target_tilename, Identifier replacement_tilename, MoneyCount base_price, ItemCount max_count):
		Item(id_, std::move(name_), base_price, max_count), targetTilename(std::move(target_tilename)), replacementTilename(std::move(replacement_tilename)) {}

	bool TerrainSeed::use(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers, std::pair<float, float>) {
		Realm &realm = *place.realm;
		Tileset &tileset = realm.getTileset();

		std::optional<TileID> tile = realm.tryTile(Layer::Terrain, place.position);
		if (!tile)
			return false;

		bool match{};

		if (targetTilename.getPathStart() == "category")
			match = tileset.isInCategory(*tile, targetTilename);
		else
			match = tileset[*tile] == targetTilename;

		if (match) {
			place.player->getInventory(0)->decrease(stack, slot, 1, true);
			place.set(Layer::Terrain, replacementTilename);
			return true;
		}

		return false;
	}

	bool TerrainSeed::drag(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets, DragAction) {
		return use(slot, stack, place, modifiers, offsets);
	}
}
