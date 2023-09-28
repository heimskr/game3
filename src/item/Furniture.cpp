#include "MarchingSquares.h"
#include "Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "item/Furniture.h"
#include "realm/Realm.h"
#include "tileentity/Ghost.h"

namespace Game3 {
	bool Furniture::use(Slot slot, ItemStack &stack, const Place &place, Modifiers, std::pair<float, float>) {
		if (auto id = place.get(getLayer()); !id || *id != place.realm->getTileset().getEmptyID())
			return false;

		if (apply(place)) {
			const InventoryPtr inventory = place.player->getInventory();
			if (--stack.count == 0)
				inventory->erase(slot);
			inventory->notifyOwner();
			return true;
		}

		return false;
	}

	std::shared_ptr<Furniture> create(ItemID id_, std::string name_, MoneyCount base_price, Layer layer, Identifier tilename) {
		return std::make_shared<SimpleFurniture>(std::move(id_), std::move(name_), base_price, layer, std::move(tilename));
	}

	std::shared_ptr<Furniture> create(ItemID id_, std::string name_, MoneyCount base_price, Layer layer, MarchableInfo info) {
		return std::make_shared<MarchableFurniture>(std::move(id_), std::move(name_), base_price, layer, std::move(info));
	}

	SimpleFurniture::SimpleFurniture(ItemID id_, std::string name_, MoneyCount base_price, Layer layer_, Identifier tilename_):
		Furniture(std::move(id_), std::move(name_), base_price, 64),
		tilename(std::move(tilename_)),
		layer(layer_) {}

	bool SimpleFurniture::apply(const Place &place) {
		place.set(layer, tilename);
		return true;
	}

	MarchableFurniture::MarchableFurniture(ItemID id_, std::string name_, MoneyCount base_price, Layer layer_, MarchableInfo info_):
		Furniture(std::move(id_), std::move(name_), base_price, 64),
		info(std::move(info_)),
		layer(layer_) {}

	bool MarchableFurniture::apply(const Place &place) {
		place.set(layer, march(place));
		return true;
	}

	TileID MarchableFurniture::march(const Place &place) {
		const Tileset &tileset = place.realm->getTileset();

		const auto march_result = march4([&](int8_t row_offset, int8_t column_offset) -> bool {
			const Position offset_position = place.position + Position(row_offset, column_offset);
			const TileID tile = place.realm->getTile(layer, offset_position);
			for (const Identifier &category: info.categories)
				if (tileset.isInCategory(tile, category))
					return true;
			return false;
		});

		return tileset[info.corner] + march_result;
	}
}
