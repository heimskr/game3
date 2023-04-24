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

		protected:
			Sheep(): Animal(ID()) {}
	};
}
