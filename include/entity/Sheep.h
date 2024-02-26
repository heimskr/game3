#pragma once

#include "entity/Animal.h"

namespace Game3 {
	class Building;

	class Sheep: public Animal {
		public:
			static Identifier ID() { return {"base", "entity/sheep"}; }

			static std::shared_ptr<Sheep> create(const std::shared_ptr<Game> &) {
				return Entity::create<Sheep>();
			}

			static std::shared_ptr<Sheep> fromJSON(const std::shared_ptr<Game> &game, const nlohmann::json &json) {
				auto out = Entity::create<Sheep>();
				out->absorbJSON(game, json);
				return out;
			}

			std::string getName() const override { return "Sheep"; }

			Identifier getMilk() const override { return {"base", "fluid/milk"}; }

			std::vector<ItemStack> getDrops() override {
				std::vector<ItemStack> out = Animal::getDrops();
				out.emplace_back(getGame(), "base:item/raw_meat");
				return out;
			}

			friend class Entity;

		protected:
			Sheep():
				Entity(ID()), Animal() {}
	};
}
