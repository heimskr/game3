#include "MarchingSquares.h"
#include "Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "item/Furniture.h"
#include "realm/Realm.h"

namespace Game3 {
	bool Furniture::preCheck(const Place &place) const {
		if (auto id = place.get(getLayer()); !id || *id != place.realm->getTileset().getEmptyID())
			return false;
		return true;
	}

	bool Furniture::use(Slot slot, ItemStack &stack, const Place &place, Modifiers, std::pair<float, float>) {
		if (!preCheck(place))
			return false;

		if (apply(place)) {
			const InventoryPtr inventory = place.player->getInventory();
			auto inventory_lock = inventory->uniqueLock();
			if (--stack.count == 0)
				inventory->erase(slot);
			inventory->notifyOwner();
			return true;
		}

		return false;
	}

	bool Furniture::drag(Slot slot, ItemStack &stack, const Place &place, Modifiers modifiers) {
		return use(slot, stack, place, modifiers, {0.f, 0.f});
	}

	std::shared_ptr<Furniture> Furniture::createSimple(ItemID id, std::string name, MoneyCount base_price, Layer layer, Identifier tilename) {
		return std::make_shared<SimpleFurniture>(std::move(id), std::move(name), base_price, layer, std::move(tilename));
	}

	std::shared_ptr<Furniture> Furniture::createMarchable(ItemID id, std::string name, MoneyCount base_price, Layer layer, Identifier start, Identifier autotile) {
		return std::make_shared<MarchableFurniture>(std::move(id), std::move(name), base_price, layer, std::move(start), std::move(autotile));
	}

	std::shared_ptr<Furniture> Furniture::createCustom(ItemID id, std::string name, MoneyCount base_price, std::function<bool(const Place &)> placer) {
		return std::make_shared<CustomFurniture>(std::move(id), std::move(name), base_price, std::move(placer));
	}

	std::shared_ptr<Furniture> Furniture::createTileEntity(ItemID id, std::string name, MoneyCount base_price, std::function<bool(const Place &)> placer) {
		return std::make_shared<TileEntityFurniture>(std::move(id), std::move(name), base_price, std::move(placer));
	}

	SimpleFurniture::SimpleFurniture(ItemID id_, std::string name_, MoneyCount base_price, Layer layer_, Identifier tilename_):
		Furniture(std::move(id_), std::move(name_), base_price, 64),
		tilename(std::move(tilename_)),
		layer(layer_) {}

	bool SimpleFurniture::apply(const Place &place) {
		place.set(layer, tilename);
		return true;
	}

	MarchableFurniture::MarchableFurniture(ItemID id_, std::string name_, MoneyCount base_price, Layer layer_, Identifier start_, Identifier autotile_):
		Furniture(std::move(id_), std::move(name_), base_price, 64),
		start(std::move(start_)),
		autotile(std::move(autotile_)),
		layer(layer_) {}

	bool MarchableFurniture::apply(const Place &place) {
		place.set(layer, march(place));
		return true;
	}

	TileID MarchableFurniture::march(const Place &place) {
		const Tileset &tileset = place.realm->getTileset();

		std::shared_ptr<AutotileSet> autotile_set = tileset.getAutotileSet(autotile);

		const auto march_result = march4([&](int8_t row_offset, int8_t column_offset) -> bool {
			const Position offset_position = place.position + Position(row_offset, column_offset);
			return autotile_set->members.contains(tileset[place.realm->getTile(layer, offset_position)]);
		});

		return tileset[start] + march_result;
	}

	CustomFurniture::CustomFurniture(ItemID id_, std::string name_, MoneyCount base_price, std::function<bool(const Place &)> placer_, Layer layer_):
		Furniture(std::move(id_), std::move(name_), base_price, 64),
		placer(std::move(placer_)),
		layer(layer_) {}

	bool CustomFurniture::apply(const Place &place) {
		return placer(place);
	}

	TileEntityFurniture::TileEntityFurniture(ItemID id_, std::string name_, MoneyCount base_price, std::function<bool(const Place &)> placer_):
		CustomFurniture(std::move(id_), std::move(name_), base_price, std::move(placer_), Layer::Invalid) {}

	bool TileEntityFurniture::preCheck(const Place &place) const {
		return !place.realm->tileEntityAt(place.position);
	}
}
