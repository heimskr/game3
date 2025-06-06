#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "tile/ForestFloorTile.h"
#include "types/Position.h"
#include "util/Log.h"
#include "util/Util.h"

namespace Game3 {
	ForestFloorTile::ForestFloorTile():
		Tile(ID()) {}

	bool ForestFloorTile::interact(const Place &place, Layer layer, const ItemStackPtr &used_item, Hand hand) {
		if (layer != Layer::Vegetation) {
			return false;
		}

		RealmPtr realm = place.realm;
		PlayerPtr player = place.player;
		InventoryPtr inventory = player->getInventory(0);

		assert(realm);
		assert(player);
		assert(inventory);

		auto lock = inventory->uniqueLock();

		if (player->hasTooldown()) {
			return false;
		}

		ItemStackPtr active = used_item? used_item : inventory->getActive();

		if (active && active->hasAttribute("base:attribute/shovel")) {
			player->setTooldown(1, active);

			if (active->reduceDurability()) {
				inventory->erase(used_item? player->getHeldSlot(hand) : inventory->activeSlot.load());
			}

			GamePtr game = realm->getGame();

			player->give(ItemStack::create(game, "base:item/dirt"));
			if (threadContext.random(1, 10) <= 2) {
				player->give(ItemStack::create(game, "base:item/moss"));
			}

			inventory->notifyOwner({});
			return true;
		}

		return false;
	}

	void ForestFloorTile::randomTick(const Place &place) {
		Tile::randomTick(place);

		if (threadContext.random(0, 99) != 0) {
			return;
		}

		GamePtr game = place.realm->getGame();

		// If there are any adjacent or overlapping items, give up and don't spawn anything.
		if (auto entities = place.realm->getEntities(place.position.getChunk())) {
			auto lock = entities->sharedLock();
			for (const WeakEntityPtr &weak_entity: *entities) {
				if (EntityPtr entity = weak_entity.lock()) {
					if (entity->position.taxiDistance(place.position) <= 3 && std::dynamic_pointer_cast<ItemEntity>(entity)) {
						return;
					}
				}
			}
		}

		if (!place.isPathable()) {
			return;
		}

		static const std::vector<Identifier> mushrooms{
			"base:item/saffron_milkcap",
			"base:item/saffron_milkcap",
			"base:item/saffron_milkcap",
			"base:item/honey_fungus",
			"base:item/honey_fungus",
			"base:item/brittlegill",
		};

		ItemStack::spawn(place, game, choose(mushrooms));
	}
}
