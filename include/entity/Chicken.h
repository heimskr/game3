#pragma once

#include "entity/Animal.h"

namespace Game3 {
	class Building;

	class Chicken: public Animal {
		public:
			static Identifier ID() { return {"base", "entity/chicken"}; }

			static std::shared_ptr<Chicken> create(Game &) {
				return Entity::create<Chicken>();
			}

			static std::shared_ptr<Chicken> fromJSON(Game &game, const nlohmann::json &json) {
				auto out = Entity::create<Chicken>();
				out->absorbJSON(game, json);
				return out;
			}

			std::string getName() const override { return "Chicken"; }

			std::vector<ItemStack> getDrops() override {
				std::vector<ItemStack> out = Animal::getDrops();
				out.emplace_back(getGame(), "base:item/raw_meat");
				return out;
			}

			void tick(Game &, float) override;

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
