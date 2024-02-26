#pragma once

#include "entity/Animal.h"

namespace Game3 {
	class Building;

	class Dog: public Animal {
		public:
			static Identifier ID() { return {"base", "entity/dog"}; }

			static std::shared_ptr<Dog> create(const std::shared_ptr<Game> &) {
				return Entity::create<Dog>();
			}

			static std::shared_ptr<Dog> fromJSON(const std::shared_ptr<Game> &game, const nlohmann::json &json) {
				auto out = Entity::create<Dog>();
				out->absorbJSON(game, json);
				return out;
			}

			std::string getName() const override { return "Dog"; }

			/** You do not get to kill the dog. */
			HitPoints getMaxHealth() const override { return INVINCIBLE; }

			friend class Entity;

		protected:
			Dog():
				Entity(ID()), Animal() {}
	};
}
