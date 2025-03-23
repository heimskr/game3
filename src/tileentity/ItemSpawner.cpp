#include "util/Log.h"
#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "lib/JSON.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "tileentity/ItemSpawner.h"
#include "ui/Window.h"
#include "util/Util.h"

namespace Game3 {
	ItemSpawner::ItemSpawner(Position position_, float minimum_time, float maximum_time, std::vector<ItemStackPtr> spawnables_):
		TileEntity("base:tile/empty", ID(), position_, false),
		minimumTime(minimum_time),
		maximumTime(maximum_time),
		spawnables(std::move(spawnables_)) {}

	void ItemSpawner::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		auto &object = json.as_object();
		object["minimumTime"] = minimumTime;
		object["maximumTime"] = maximumTime;
		auto &array = object["spawnables"].emplace_array();
		for (const ItemStackPtr &spawnable: spawnables) {
			array.emplace_back(boost::json::value_from(*spawnable));
		}
	}

	void ItemSpawner::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		minimumTime = getDouble(json.at("minimumTime"));
		maximumTime = getDouble(json.at("maximumTime"));
		for (const auto &spawnable: json.at("spawnables").as_array())
			spawnables.emplace_back(boost::json::value_to<ItemStackPtr>(spawnable, game));
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
			choose(spawnables)->spawn(getPlace());

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
