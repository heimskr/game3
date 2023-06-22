#pragma once

#include "entity/Animal.h"

namespace Game3 {
	class Building;

	class Dog: public Animal {
		public:
			static Identifier ID() { return {"base", "entity/dog"}; }

			static std::shared_ptr<Dog> create(Game &) {
				return Entity::create<Dog>();
			}

			static std::shared_ptr<Dog> fromJSON(Game &game, const nlohmann::json &json) {
				auto out = Entity::create<Dog>();
				out->absorbJSON(game, json);
				return out;
			}

			/** You do not get to kill the dog. */
			HitPoints maxHealth() const override { return INVINCIBLE; }

			friend class Entity;

		protected:
			Dog(): Animal(ID()) {}
	};
}
