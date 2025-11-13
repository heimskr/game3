#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/EmptyFlask.h"
#include "realm/Realm.h"

namespace Game3 {
	bool EmptyFlask::use(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers, std::pair<float, float>) {
		PlayerPtr player = place.player;
		RealmPtr  realm  = place.realm;
		GamePtr game = realm->getGame();
		assert(realm->getSide() == Side::Server);

		FluidRegistry &registry = game->registry<FluidRegistry>();

		auto yield_flask = [&](const Identifier &fluid_name) {
			const InventoryPtr inventory = player->getInventory(0);
			assert(inventory);
			inventory->decrease(stack, slot, 1, true);
			player->give(ItemStack::create(game, fluid_name, 1), slot);
			inventory->notifyOwner({});
		};

		if (std::optional<FluidTile> tile = realm->tryFluid(place.position); tile && (FluidTile::FULL <= tile->level || tile->isInfinite())) {
			std::shared_ptr<Fluid> fluid = registry.maybe(tile->id);
			if (!fluid || !fluid->flaskName) {
				return false;
			}

			if (FluidPtr resolution = fluid->resolve(place)) {
				fluid = resolution;
				tile->id = fluid->registryID;
			}

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
