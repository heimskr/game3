#include "Log.h"
#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Crop.h"
#include "game/Inventory.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "tile/CropTile.h"
#include "util/Util.h"

namespace Game3 {
	CropTile::CropTile(Identifier id, std::shared_ptr<Crop> crop_):
		Tile(std::move(id)), crop(std::move(crop_)) {}

	CropTile::CropTile(std::shared_ptr<Crop> crop_):
		Tile(ID()), crop(std::move(crop_)) {}

	void CropTile::randomTick(const Place &place) {
		Tile::randomTick(place);

		assert(!crop->stages.empty());

		auto &realm = *place.realm;
		auto &tileset = realm.getTileset();

		const auto tile = place.get(Layer::Submerged);
		if (!tile) {
			return;
		}

		const Identifier tilename = tileset[*tile];
		if (tilename && isRipe(tilename)) {
			return;
		}

		std::uniform_real_distribution distribution{0., 1.};
		if (crop->chance <= distribution(threadContext.rng)) {
			return;
		}

		auto iter = std::find(crop->stages.begin(), crop->stages.end(), tilename);
		if (iter != crop->stages.end()) {
			place.set(Layer::Submerged, *++iter);
		} else {
			WARN("Couldn't find {} in crop stages for crop {}", tilename, crop->identifier);
		}
	}

	bool CropTile::interact(const Place &place, Layer layer, const ItemStackPtr &used_item, Hand hand) {
		assert(!crop->stages.empty());

		if (auto *object = crop->customData.if_object()) {
			if (auto *partial_harvest = object->if_contains("partialHarvest")) {
				if (place.getName(layer) == boost::json::value_to<Identifier>(partial_harvest->at("full"))) {
					const InventoryPtr inventory = place.player->getInventory(0);
					auto inventory_lock = inventory->uniqueLock();
					if (!inventory->add(ItemStack::create(place.getGame(), boost::json::value_to<Identifier>(partial_harvest->at("item"))))) {
						place.set(layer, boost::json::value_to<Identifier>(partial_harvest->at("empty")));
						return true;
					}
				}
			}
		}

		if (auto tilename = place.getName(layer); tilename && isRipe(*tilename)) {
			place.set(layer, 0);
			for (const ItemStackPtr &stack: crop->products.getStacks()) {
				place.player->give(stack);
			}
			return true;
		}

		return Tile::interact(place, layer, used_item, hand);
	}

	bool CropTile::isRipe(const Identifier &tilename) const {
		return tilename == crop->stages.back();
	}
}
