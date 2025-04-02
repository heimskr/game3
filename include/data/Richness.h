#pragma once

#include "data/Identifier.h"

#include <boost/json/fwd.hpp>

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

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &, const Richness &);
		friend Richness tag_invoke(boost::json::value_to_tag<Richness>, const boost::json::value &);
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const Richness &);
	Richness tag_invoke(boost::json::value_to_tag<Richness>, const boost::json::value &);
}
