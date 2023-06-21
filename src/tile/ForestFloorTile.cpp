#include "Log.h"
#include "Position.h"
#include "entity/ItemEntity.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "tile/ForestFloorTile.h"
#include "util/Util.h"

namespace Game3 {
	ForestFloorTile::ForestFloorTile():
		Tile(ID()) {}

	void ForestFloorTile::randomTick(const Place &place) {
		auto &realm = *place.realm;
		auto &game = realm.getGame();

		static std::uniform_int_distribution distribution(0, 99);
		if (distribution(threadContext.rng) != 0)
			return;

		// If there are any adjacent or overlapping items, give up and don't spawn anything.
		if (auto entities = realm.getEntities(getChunkPosition(place.position))) {
			auto lock = entities->sharedLock();
			for (const auto &entity: *entities)
				if (entity->position.taxiDistance(place.position) <= 3 && entity->cast<ItemEntity>())
					return;
		}

		std::vector<ItemStack> mushrooms {
			{game, "base:item/saffron_milkcap"},
			{game, "base:item/saffron_milkcap"},
			{game, "base:item/saffron_milkcap"},
			{game, "base:item/honey_fungus"},
			{game, "base:item/honey_fungus"},
			{game, "base:item/brittlegill"},
		};

		choose(mushrooms, threadContext.rng).spawn(place.realm, place.position);
	}
}
