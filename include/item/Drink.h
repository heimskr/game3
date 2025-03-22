#pragma once

#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "interface/HasFluidType.h"
#include "item/Food.h"
#include "realm/Realm.h"

#include "fixed_string.hpp"

namespace Game3 {
	template <fixstr::fixed_string ID, HitPoints HP = 0>
	class Drink: public Food, public HasFluidType {
		public:
			using Food::Food;

			using Food::use;
			bool use(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float>) override {
				RealmPtr realm = place.realm;
				std::shared_ptr<Player> player = place.player;
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

			Identifier getFluidType() const override { return Identifier(ID); }

			HitPoints getHealedPoints(const std::shared_ptr<Player> &) override {
				return HP;
			}
	};
}
