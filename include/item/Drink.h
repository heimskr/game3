#pragma once

#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "interface/HasFluidType.h"
#include "item/Food.h"
#include "realm/Realm.h"

namespace Game3 {
	template <HitPoints HP = 0>
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

			HitPoints getHealedPoints(const std::shared_ptr<Player> &) override {
				return HP;
			}
	};

	class Mead: public Drink<> {
		public:
			using Drink<>::Drink;

			Identifier getFluidType() const override { return Identifier("base:fluid/mead"); }

			HitPoints getHealedPoints(const std::shared_ptr<Player> &player) override {
				return (player->getMaxHealth() - player->getHealth()) / 4;
			}
	};

	class Milk: public Drink<12> {
		public:
			using Drink<12>::Drink;

			Identifier getFluidType() const override { return Identifier("base:fluid/milk"); }
	};
}
