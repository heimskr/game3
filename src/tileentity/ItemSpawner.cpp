#include "Log.h"
#include "Tileset.h"
#include "threading/ThreadContext.h"
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
		TileEntity("base:tile/empty", ID(), std::move(position_), false),
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

	void ItemSpawner::tick(Game &, float delta) {
		if (getSide() != Side::Server)
			return;

		static std::uniform_real_distribution distribution(0., 1.);
		for (float i = 0.f; i < delta; i += .1f) {
			if (distribution(threadContext.rng) < chancePerTenth) {
				for (const auto &entity: getRealm()->findEntities(getPosition()))
					if (entity->is("base:entity/item"))
						return;
				choose(spawnables).spawn(getRealm(), getPosition());
				return;
			}
		}
	}

	void ItemSpawner::render(SpriteRenderer &) {}

	void ItemSpawner::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << chancePerTenth;
		buffer << spawnables;
	}

	void ItemSpawner::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		buffer >> chancePerTenth;
		buffer >> spawnables;
	}
}
