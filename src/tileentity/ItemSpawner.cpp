#include "Log.h"
#include "graphics/Tileset.h"
#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "graphics/SpriteRenderer.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "tileentity/ItemSpawner.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/tab/InventoryTab.h"
#include "util/Util.h"

namespace Game3 {
	ItemSpawner::ItemSpawner(Position position_, float minimum_time, float maximum_time, std::vector<ItemStack> spawnables_):
		TileEntity("base:tile/empty", ID(), position_, false),
		minimumTime(minimum_time),
		maximumTime(maximum_time),
		spawnables(std::move(spawnables_)) {}

	void ItemSpawner::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["minimumTime"] = minimumTime;
		json["maximumTime"] = maximumTime;
		json["spawnables"]  = spawnables;
	}

	void ItemSpawner::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		minimumTime = json.at("minimumTime");
		maximumTime = json.at("maximumTime");
		for (const auto &spawnable: json.at("spawnables"))
			spawnables.emplace_back(ItemStack::fromJSON(game, spawnable));
	}

	void ItemSpawner::tick(const TickArgs &args) {
		if (getSide() != Side::Server)
			return;

		Ticker ticker{*this, args};

		bool can_spawn = true;

		for (const auto &entity: getRealm()->findEntities(getPosition())) {
			if (entity->is("base:entity/item")) {
				can_spawn = false;
				break;
			}
		}

		if (can_spawn)
			choose(spawnables).spawn(getRealm(), getPosition());

		std::uniform_real_distribution distribution{minimumTime, maximumTime};
		enqueueTick(std::chrono::microseconds(int64_t(1e6 * distribution(threadContext.rng))));
	}

	void ItemSpawner::render(SpriteRenderer &) {}

	void ItemSpawner::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << minimumTime;
		buffer << maximumTime;
		buffer << spawnables;
	}

	void ItemSpawner::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		buffer >> minimumTime;
		buffer >> maximumTime;
		buffer >> spawnables;
	}
}
