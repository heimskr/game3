#pragma once

#include "entity/Animal.h"

namespace Game3 {
	class Building;

	class Chicken: public Animal {
		public:
			static Identifier ID() { return {"base", "entity/chicken"}; }

			static std::shared_ptr<Chicken> create(Game &game) {
				auto out = std::shared_ptr<Chicken>(new Chicken());
				out->init(game);
				return out;
			}

			static std::shared_ptr<Chicken> fromJSON(Game &game, const nlohmann::json &json) {
				auto out = Entity::create<Chicken>();
				out->absorbJSON(game, json);
				return out;
			}

			friend class Entity;

		protected:
			Chicken(): Animal(ID()) {}
	};
}
