#pragma once

#include "entity/Animal.h"

namespace Game3 {
	class Building;

	class Pig: public Animal {
		public:
			static Identifier ID() { return {"base", "entity/pig"}; }

			static std::shared_ptr<Pig> create(Game &game) {
				auto out = std::shared_ptr<Pig>(new Pig());
				out->init(game);
				return out;
			}

		protected:
			Pig(): Animal(ID()) {}
	};
}
