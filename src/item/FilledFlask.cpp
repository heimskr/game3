#include "Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/FilledFlask.h"
#include "realm/Realm.h"

namespace Game3 {
	bool FilledFlask::use(Slot slot, ItemStack &stack, const Place &place, Modifiers, std::pair<float, float>) {
		Realm  &realm  = *place.realm;
		assert(realm.getSide() == Side::Server);
		Game   &game   = realm.getGame();
		Player &player = *place.player;

		if (std::optional<FluidTile> tile = realm.tryFluid(place.position); !tile || tile->level == 0) {
			std::shared_ptr<Fluid> fluid = game.registry<FluidRegistry>().at(fluidName);
			if (!fluid)
				return false;

			realm.setFluid(place.position, FluidTile(fluid->registryID, FluidTile::FULL));

			const InventoryPtr inventory = player.getInventory();

			{
				auto lock = inventory->uniqueLock();
				if (--stack.count == 0)
					inventory->erase(slot);
			}

			player.give(ItemStack(realm.getGame(), "base:item/flask", 1), slot);
			inventory->notifyOwner();
			return true;
		}

		return false;
	}

	FluidStack FilledFlask::getFluidStack(const Game &game) const {
		return getFluidStack(game.registry<FluidRegistry>());
	}

	FluidStack FilledFlask::getFluidStack(const FluidRegistry &registry) const {
		std::shared_ptr<Fluid> fluid = registry.at(fluidName);
		if (!fluid)
			throw std::runtime_error("Couldn't find fluid " + std::string(fluidName));
		return FluidStack(fluid->registryID, FluidTile::FULL);
	}
}
