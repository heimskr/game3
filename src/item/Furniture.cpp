#include "algorithm/MarchingSquares.h"
#include "data/SoundSet.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "graphics/Tileset.h"
#include "item/Furniture.h"
#include "realm/Realm.h"
#include "tileentity/CraftingStation.h"
#include "types/Position.h"

namespace Game3 {
	bool Furniture::preCheck(const Place &place) const {
		if (auto id = place.get(getLayer()); !id || *id != place.realm->getTileset().getEmptyID())
			return false;
		return true;
	}

	Identifier Furniture::getSoundSet() const {
		return "base:sound_set/wood_place";
	}

	bool Furniture::use(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers, std::pair<float, float>) {
		if (!preCheck(place))
			return false;

		if (apply(place)) {
			InventoryPtr inventory = place.player->getInventory(0);
			if (SoundSetPtr sound_set = place.getGame()->registry<SoundSetRegistry>().maybe(getSoundSet())) {
				place.realm->playSound(place.position, sound_set->choose(), sound_set->choosePitch(), 32);
			}
			assert(inventory);
			inventory->decrease(stack, slot, 1, true);
			return true;
		}

		return false;
	}

	bool Furniture::drag(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets) {
		return use(slot, stack, place, modifiers, offsets);
	}

	std::shared_ptr<Furniture> Furniture::createSimple(ItemID id, std::string name, MoneyCount base_price, Layer layer, Identifier tilename, Identifier sound_set_id) {
		return std::make_shared<SimpleFurniture>(std::move(id), std::move(name), base_price, layer, std::move(tilename), std::move(sound_set_id));
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

	std::shared_ptr<Furniture> Furniture::createStation(ItemID id, std::string name, MoneyCount base_price, Identifier tilename, Identifier station_name) {
		return std::make_shared<StationFurniture>(std::move(id), std::move(name), base_price, std::move(tilename), std::move(station_name));
	}

	SimpleFurniture::SimpleFurniture(ItemID id, std::string name, MoneyCount basePrice, Layer layer, Identifier tilename, Identifier soundSetID):
		Furniture(std::move(id), std::move(name), basePrice, 64),
		tilename(std::move(tilename)),
		soundSetID(std::move(soundSetID)),
		layer(layer) {}

	bool SimpleFurniture::apply(const Place &place) {
		place.set(layer, tilename);
		return true;
	}

	Identifier SimpleFurniture::getSoundSet() const {
		if (soundSetID) {
			return soundSetID;
		}

		return Furniture::getSoundSet();
	}

	MarchableFurniture::MarchableFurniture(ItemID id, std::string name, MoneyCount basePrice, Layer layer, Identifier start, Identifier autotile):
		Furniture(std::move(id), std::move(name), basePrice, 64),
		start(std::move(start)),
		autotile(std::move(autotile)),
		layer(layer) {}

	bool MarchableFurniture::apply(const Place &place) {
		place.set(layer, march(place));
		return true;
	}

	TileID MarchableFurniture::march(const Place &place) {
		const Tileset &tileset = place.realm->getTileset();

		std::shared_ptr<AutotileSet> autotile_set = tileset.getAutotileSet(autotile);
		assert(autotile_set != nullptr);

		const auto march_result = march4([&](int8_t row_offset, int8_t column_offset) -> bool {
			const Position offset_position = place.position + Position(row_offset, column_offset);
			return autotile_set->members.contains(tileset[place.realm->getTile(layer, offset_position)]);
		});

		return tileset[start] + march_result;
	}

	CustomFurniture::CustomFurniture(ItemID id, std::string name, MoneyCount basePrice, std::function<bool(const Place &)> placer, Layer layer):
		Furniture(std::move(id), std::move(name), basePrice, 64),
		placer(std::move(placer)),
		layer(layer) {}

	bool CustomFurniture::apply(const Place &place) {
		return placer(place);
	}

	TileEntityFurniture::TileEntityFurniture(ItemID id, std::string name, MoneyCount basePrice, std::function<bool(const Place &)> placer):
		CustomFurniture(std::move(id), std::move(name), basePrice, std::move(placer), Layer::Invalid) {}

	bool TileEntityFurniture::preCheck(const Place &place) const {
		return !place.realm->tileEntityAt(place.position);
	}

	bool StationFurniture::preCheck(const Place &place) const {
		return !place.realm->tileEntityAt(place.position);
	}

	StationFurniture::StationFurniture(ItemID item_id, std::string name, MoneyCount basePrice, Identifier tilename, Identifier stationType):
		Furniture(std::move(item_id), std::move(name), basePrice, 64),
		tilename(std::move(tilename)),
		stationType(std::move(stationType)) {}

	bool StationFurniture::apply(const Place &place) {
		return TileEntity::spawn<CraftingStation>(place, tilename, place.position, stationType, identifier) != nullptr;
	}
}
