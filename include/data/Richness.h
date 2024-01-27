#pragma once

#include "data/Identifier.h"

#include <nlohmann/json_fwd.hpp>

#include <map>
#include <optional>

namespace Game3 {
	class Game;

	class Richness {
		public:
			Richness() = default;

			std::optional<double> operator[](const Identifier &) const;

			static Richness makeRandom(const Game &);

			inline auto begin() { return richnesses.begin(); }
			inline auto end()   { return richnesses.end();   }
			inline auto begin() const { return richnesses.begin(); }
			inline auto end()   const { return richnesses.end();   }

		private:
			std::map<Identifier, double> richnesses;


		friend void to_json(nlohmann::json &, const Richness &);
		friend void from_json(const nlohmann::json &, Richness &);
	};

	void to_json(nlohmann::json &, const Richness &);
	void from_json(const nlohmann::json &, Richness &);
}
