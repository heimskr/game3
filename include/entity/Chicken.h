#pragma once

#include "entity/Animal.h"

namespace Game3 {
	class Chicken: public Animal {
		public:
			static Identifier ID() { return {"base", "entity/chicken"}; }

			static std::shared_ptr<Chicken> create(const std::shared_ptr<Game> &) {
				return Entity::create<Chicken>();
			}

			std::string getName() const override { return "Chicken"; }

			std::vector<ItemStackPtr> getDrops() override {
				std::vector<ItemStackPtr> out = Animal::getDrops();
				out.emplace_back(ItemStack::create(getGame(), "base:item/raw_meat"));
				return out;
			}

			void tick(const TickArgs &) override;

		protected:
			Chicken():
				Entity(ID()), Animal() {}

		private:
			bool firstEgg = true;
			Tick eggTick{};

			void layEgg();

		friend class Entity;
	};
}
