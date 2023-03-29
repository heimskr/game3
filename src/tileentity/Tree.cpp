#include "Tiles.h"
#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "realm/Realm.h"
#include "tileentity/Tree.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/SpriteRenderer.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	void Tree::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["immatureID"] = immatureID;
		json["age"] = age;
		json["hiveAge"] = hiveAge;
	}

	void Tree::absorbJSON(const nlohmann::json &json) {
		TileEntity::absorbJSON(json);
		immatureID = json.at("immatureID");
		age = json.at("age");
		hiveAge = json.at("hiveAge");
	}

	void Tree::init(std::default_random_engine &rng) {
		if (rng() % 10 == 0)
			hiveAge = 0.f;
	}

	void Tree::tick(Game &, float delta) {
		age += delta;
		if (0.f <= hiveAge && hiveAge < HIVE_MATURITY)
			hiveAge += delta;
	}

	bool Tree::onInteractNextTo(const std::shared_ptr<Player> &player) {
		if (age < MATURITY)
			return false;

		auto &inventory = *player->inventory;
		const Slot active_slot = inventory.activeSlot;
		if (auto *active_stack = inventory[active_slot]) {
			if (active_stack->has(ItemAttribute::Axe)) {
				auto &realm = *getRealm();
				if (!inventory.add({Item::WOOD, 1})) {
					realm.queueRemoval(shared_from_this());
					if (active_stack->reduceDurability())
						inventory.erase(active_slot);
					ItemCount saplings = 1;
					while (rand() % 4 == 1)
						++saplings;
					auto leftover = inventory.add({Item::SAPLING, saplings});
					if (leftover)
						realm.spawn<ItemEntity>(player->position, *leftover);
					inventory.notifyOwner();
					return true;
				}
				return false;
			}
		}

		if (HIVE_MATURITY <= hiveAge && !inventory.add({Item::HONEY, 1})) {
			hiveAge = 0.f;
			inventory.notifyOwner();
			return true;
		}

		return false;
	}

	void Tree::render(SpriteRenderer &sprite_renderer) {
		if (!isVisible())
			return;
		auto realm = getRealm();
		if (tileID != tileSets.at(realm->type)->getEmpty()) {
			auto &tilemap = *realm->tilemap2;
			const auto tilesize = tilemap.tileSize;
			TileID tile_id = age < MATURITY? immatureID : tileID;
			if (tile_id != immatureID) {
				if (0.f <= hiveAge)
					tile_id += 4;
				if (HIVE_MATURITY <= hiveAge)
					tile_id += 3;
			}
			const auto x = (tile_id % (tilemap.setWidth / tilesize)) * tilesize;
			const auto y = (tile_id / (tilemap.setWidth / tilesize)) * tilesize;
			sprite_renderer.drawOnMap(tilemap.texture, position.column, position.row, x / 2, y / 2, tilesize, tilesize);
		}
	}
}
