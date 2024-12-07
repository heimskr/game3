#include "entity/EntityFactory.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "item/EntitySpawnItem.h"
#include "realm/Realm.h"

namespace Game3 {
	EntitySpawnItem::EntitySpawnItem(ItemID id_, std::string name_, MoneyCount base_price, Identifier entity_id, ItemCount max_count):
		Item(std::move(id_), std::move(name_), base_price, max_count),
		entityID(std::move(entity_id)) {}

	bool EntitySpawnItem::use(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers, std::pair<float, float>) {
		RealmPtr realm = place.realm;
		GamePtr game = realm->getGame();

		if (game->getSide() != Side::Server)
			return false;

		auto factory = game->registry<EntityFactoryRegistry>().maybe(entityID);

		if (!factory) {
			ERROR("Couldn't spawn entity of type {}", entityID);
			return true;
		}

		EntityPtr entity = (*factory)(game);
		entity->spawning = true;

		if (!entity->canSpawnAt(place))
			return true;

		entity->setRealm(realm);
		realm->queueEntityInit(std::move(entity), place.position);

		InventoryPtr inventory = place.player->getInventory(0);
		assert(inventory);
		inventory->decrease(stack, slot, 1, true);

		return true;
	}
}
