#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/Mead.h"
#include "realm/Realm.h"

namespace Game3 {
	bool Mead::use(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float>) {
		RealmPtr realm = place.realm;
		PlayerPtr player = place.player;
		GamePtr game = realm->getGame();
		assert(game->getSide() == Side::Server);

		if (place.position == player->getPosition()) {
			return use(slot, stack, player, modifiers);
		}

		if (std::optional<FluidTile> tile = realm->tryFluid(place.position); !tile || tile->level == 0) {
			std::shared_ptr<Fluid> fluid = game->registry<FluidRegistry>().at(getFluidType());
			if (!fluid) {
				return false;
			}

			realm->setFluid(place.position, FluidTile(fluid->registryID, FluidTile::FULL));
			player->getInventory(0)->decrease(stack, slot, 1, true);
			return true;
		}

		return false;
	}

	HitPoints Mead::getHealedPoints(const PlayerPtr &player) {
		return (player->getMaxHealth() - player->getHealth()) / 4;
	}
}
