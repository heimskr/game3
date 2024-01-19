#pragma once

#include "data/Identifier.h"

#include <map>
#include <optional>

namespace Game3 {
	class Game;

	class Richness {
		public:
			Richness() = default;

			std::optional<double> operator[](const Identifier &);

			static Richness makeRandom(const Game &);

		private:
			std::map<Identifier, double> richnesses;
	};
}
