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
		TileEntity(Monomap::VOID, TileEntity::ITEM_SPAWNER, std::move(position_), false),
		chancePerTenth(chance_per_tenth),
		spawnables(std::move(spawnables_)) {}

	void ItemSpawner::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["chance"] = chancePerTenth;
		json["spawnables"] = spawnables;
	}

	void ItemSpawner::absorbJSON(const nlohmann::json &json) {
		TileEntity::absorbJSON(json);
		chancePerTenth = json.at("chance");
		spawnables = json.at("spawnables");
	}

	void ItemSpawner::init(std::default_random_engine &) {}

	void ItemSpawner::tick(Game &game, float delta) {
		static std::uniform_real_distribution distribution(0., 1.);
		for (float i = 0; i < delta; i += 0.1) {
			if (distribution(game.dynamicRNG) < chancePerTenth) {
				for (const auto &entity: getRealm()->findEntities(getPosition()))
					if (entity->type == Entity::ITEM_TYPE)
						return;
				choose(spawnables).spawn(getRealm(), getPosition());
				return;
			}
		}
	}

	void ItemSpawner::render(SpriteRenderer &) {}
}
