#pragma once

#include "entity/Animal.h"

namespace Game3 {
	class Quarter: public Animal {
		public:
			static Identifier ID() { return {"base", "entity/quarter"}; }

			static std::shared_ptr<Quarter> create(const std::shared_ptr<Game> &) {
				return Entity::create<Quarter>();
			}

			std::string getName() const override { return "Chicken"; } // gottem

			void onSpawn() override;
			HitPoints getMaxHealth() const override { return 1000; }

			std::vector<ItemStackPtr> getDrops() override {
				std::vector<ItemStackPtr> out = Animal::getDrops();
				out.emplace_back(ItemStack::create(getGame(), "base:item/raw_meat"));
				return out;
			}

			void tick(const TickArgs &) override;

		protected:
			Quarter():
				Entity(ID()), Animal() {}

		friend class Entity;
	};
}
