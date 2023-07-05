#include "threading/ThreadContext.h"
#include "Tileset.h"
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
	Tree::Tree(Identifier tilename, Identifier immature_tilename, Position position_, float age_):
		TileEntity(std::move(tilename), ID(), std::move(position_), true),
		age(age_),
		immatureTilename(std::move(immature_tilename)) {}

	void Tree::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["immatureTilename"] = immatureTilename;
		json["age"] = age;
		json["hiveAge"] = hiveAge;
	}

	void Tree::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		immatureTilename = json.at("immatureTilename");
		age = json.at("age");
		hiveAge = json.at("hiveAge");
	}

	void Tree::onSpawn() {
		auto &tileset = getRealm()->getTileset();

		if (!tileset.isInCategory(tileID, "base:category/honey_trees"_id))
			return;

		if (threadContext.random(0, 10) == 0)
			hiveAge = 0.f;
	}

	void Tree::tick(Game &, float delta) {
		age += delta;
		if (0.f <= hiveAge && hiveAge < HIVE_MATURITY) {
			hiveAge += delta;
			increaseUpdateCounter();
		}
	}

	bool Tree::onInteractNextTo(const std::shared_ptr<Player> &player) {
		if (age < MATURITY)
			return false;

		auto realm = getRealm();
		auto &game = realm->getGame();
		auto &inventory = *player->inventory;
		const Slot active_slot = inventory.activeSlot;

		if (auto *active_stack = inventory[active_slot]) {
			if (active_stack->hasAttribute("base:attribute/axe"_id)) {
				if (!inventory.add({game, "base:item/wood"_id, 1})) {
					realm->queueDestruction(shared_from_this());
					if (active_stack->reduceDurability())
						inventory.erase(active_slot);
					ItemCount saplings = 1;
					while (rand() % 4 == 1)
						++saplings;
					player->give({game, "base:item/sapling"_id, saplings});
					increaseUpdateCounter();
					return true;
				}
				return false;
			}
		}

		if (HIVE_MATURITY <= hiveAge && !inventory.add({game, "base:item/honey"_id, 1})) {
			hiveAge = 0.f;
			inventory.notifyOwner();
			increaseUpdateCounter();
			return true;
		}

		return false;
	}

	bool Tree::hasHive() const {
		return 0.f <= hiveAge;
	}

	bool Tree::kill() {
		auto realm = getRealm();

		static const Identifier expected("base", "tileset/monomap");

		if (realm->getTileset().identifier != expected)
			return true;

		static std::uniform_real_distribution one(0., 1.);

		if (one(threadContext.rng) < CHAR_CHANCE)
			realm->setTile(Layer::Objects, getPosition(), "base:tile/charred_stump"_id);
		else
			realm->spawn<ItemEntity>(getPosition(), ItemStack(realm->getGame(), "base:item/wood"_id, 1));

		realm->setTile(Layer::Submerged, getPosition(), "base:tile/ash"_id);

		increaseUpdateCounter();
		return true;
	}

	void Tree::render(SpriteRenderer &sprite_renderer) {
		if (!isVisible())
			return;

		auto realm = getRealm();
		auto &tileset = realm->getTileset();
		if (tileID != tileset.getEmpty()) {
			const auto tilesize = tileset.getTileSize();
			TileID tile_id = tileset[age < MATURITY? immatureTilename : tileID];
			if (tile_id != getImmatureTileID(tileset)) {
				if (0.f <= hiveAge)
					tile_id += 4;
				if (HIVE_MATURITY <= hiveAge)
					tile_id += 3;
			}
			const auto texture = tileset.getTexture(realm->getGame());
			const auto x = (tile_id % (*texture->width / tilesize)) * tilesize;
			const auto y = (tile_id / (*texture->width / tilesize)) * tilesize;
			sprite_renderer(*texture, {
				.x = static_cast<float>(position.column),
				.y = static_cast<float>(position.row),
				.x_offset = static_cast<float>(x) / 2.f,
				.y_offset = static_cast<float>(y) / 2.f,
				.size_x = static_cast<float>(tilesize),
				.size_y = static_cast<float>(tilesize),
			});
		}
	}

	TileID Tree::getImmatureTileID(const Tileset &tileset) {
		return immatureTileID? *immatureTileID : *(immatureTileID = tileset[immatureTilename]);
	}

	void Tree::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << age;
		buffer << hiveAge;
		buffer << immatureTilename;
	}

	void Tree::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		buffer >> age;
		buffer >> hiveAge;
		buffer >> immatureTilename;
	}
}
