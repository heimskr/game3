#pragma once

#include "entity/Animal.h"

namespace Game3 {
	class Building;

	class Sheep: public Animal {
		public:
			static Identifier ID() { return {"base", "entity/sheep"}; }

			static std::shared_ptr<Sheep> create(Game &game) {
				auto out = std::shared_ptr<Sheep>(new Sheep());
				out->init(game);
				return out;
			}

			static std::shared_ptr<Sheep> fromJSON(Game &game, const nlohmann::json &json) {
				auto out = Entity::create<Sheep>();
				out->absorbJSON(game, json);
				return out;
			}

			friend class Entity;

		protected:
			Sheep(): Animal(ID()) {}
	};
}
