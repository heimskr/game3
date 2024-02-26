#pragma once

#include "entity/Animal.h"

namespace Game3 {
	class Building;

	class Pig: public Animal {
		public:
			static Identifier ID() { return {"base", "entity/pig"}; }

			static std::shared_ptr<Pig> create(const std::shared_ptr<Game> &) {
				return Entity::create<Pig>();
			}

			static std::shared_ptr<Pig> fromJSON(const std::shared_ptr<Game> &game, const nlohmann::json &json) {
				auto out = Entity::create<Pig>();
				out->absorbJSON(game, json);
				return out;
			}

			std::string getName() const override { return "Pig"; }

			std::vector<ItemStack> getDrops() override {
				std::vector<ItemStack> out = Animal::getDrops();
				out.emplace_back(getGame(), "base:item/raw_meat");
				return out;
			}

			friend class Entity;

		protected:
			Pig():
				Entity(ID()), Animal() {}
	};
}
