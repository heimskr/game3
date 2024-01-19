#pragma once

#include "data/Identifier.h"

#include <map>

namespace Game3 {
	class Game;

	class Richness {
		public:
			Richness() = default;

			static Richness getRandom(const Game &);

		private:
			std::map<Identifier, double> richnesses;
	};
}
