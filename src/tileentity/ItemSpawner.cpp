#include "Tileset.h"
#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "realm/Realm.h"
#include "tileentity/ItemSpawner.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/SpriteRenderer.h"
#include "ui/tab/InventoryTab.h"
#include "util/Util.h"

namespace Game3 {
	ItemSpawner::ItemSpawner(Position position_, float chance_per_tenth, std::vector<ItemStack> spawnables_):
		TileEntity("base:tile/empty", "base:te/item_spawner", std::move(position_), false),
		chancePerTenth(chance_per_tenth),
		spawnables(std::move(spawnables_)) {}

	void ItemSpawner::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["chance"] = chancePerTenth;
		json["spawnables"] = spawnables;
	}

	void ItemSpawner::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		chancePerTenth = json.at("chance");
		for (const auto &spawnable: json.at("spawnables"))
			spawnables.push_back(ItemStack::fromJSON(game, spawnable));
	}

	void ItemSpawner::init(Game &, std::default_random_engine &) {}

	void ItemSpawner::tick(Game &game, float delta) {
		static std::uniform_real_distribution distribution(0., 1.);
		for (float i = 0; i < delta; i += 0.1) {
			if (distribution(game.dynamicRNG) < chancePerTenth) {
				for (const auto &entity: getRealm()->findEntities(getPosition()))
					if (entity->is("base:entity/item"))
						return;
				choose(spawnables).spawn(getRealm(), getPosition());
				return;
			}
		}
	}

	void ItemSpawner::render(SpriteRenderer &) {}
}
