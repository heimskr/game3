#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/EmptyFlask.h"
#include "realm/Realm.h"

namespace Game3 {
	bool EmptyFlask::use(Slot slot, ItemStack &stack, const Place &place, Modifiers, std::pair<float, float>) {
		PlayerPtr player = place.player;
		RealmPtr  realm  = place.realm;
		Game &game = realm->getGame();
		assert(realm->getSide() == Side::Server);

		FluidRegistry &registry = game.registry<FluidRegistry>();

		auto yield_flask = [&](const Identifier &fluid_name) {
			const InventoryPtr inventory = player->getInventory();

			{
				auto lock = inventory->uniqueLock();
				if (--stack.count == 0)
					inventory->erase(slot);
			}

			player->give(ItemStack(game, fluid_name, 1), slot);
			inventory->notifyOwner();
		};

		if (std::optional<FluidTile> tile = realm->tryFluid(place.position); tile && (FluidTile::FULL <= tile->level || tile->isInfinite())) {
			std::shared_ptr<Fluid> fluid = registry.maybe(tile->id);
			if (!fluid || !fluid->flaskName)
				return false;

			if (!tile->isInfinite()) {
				tile->level = 0;
				realm->setFluid(place.position, *tile);
			}

			yield_flask(fluid->flaskName);
			return true;
		}

		for (const EntityPtr &entity: realm->findEntities(place.position)) {
			if (Identifier fluid_name = entity->getMilk()) {
				std::shared_ptr<Fluid> fluid = registry.maybe(fluid_name);
				if (!fluid || !fluid->flaskName)
					continue;

				yield_flask(fluid->flaskName);
				return true;
			}
		}

		return false;
	}
}
