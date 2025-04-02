#pragma once

#include "item/Item.h"
#include "registry/Registry.h"
#include "types/Types.h"

#include <boost/json/fwd.hpp>

#include <limits>
#include <optional>
#include <set>
#include <utility>
#include <vector>

namespace Game3 {
	class Game;

	class ConsumptionRule: public Registerable {
		public:
			using Registerable::Registerable;
			ConsumptionRule(const std::shared_ptr<Game> &, const boost::json::value &);

			inline const auto & getInput() const { return input; }
			inline const auto & getLaborRange() const { return laborRange; }
			inline auto getLaborOut() const { return laborOut; }
			inline auto getAlways() const { return always; }
			inline auto getRate() const { return rate; }
			inline auto getIgnoreLabor() const { return ignoreLabor; }

			static ConsumptionRule fromJSON(const std::shared_ptr<Game> &, const boost::json::value &);

		private:
			Identifier input;
			LaborAmount laborOut{};
			bool always{};
			std::pair<double, double> laborRange{-std::numeric_limits<double>::infinity(), 100};
			double rate = 1.0;
			bool ignoreLabor = false;
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const ConsumptionRule &);
	ConsumptionRule tag_invoke(boost::json::value_to_tag<ConsumptionRule>, const boost::json::value &, const GamePtr &);

	struct ConsumptionRuleRegistry: UnnamedJSONRegistry<ConsumptionRule> {
		static Identifier ID() { return {"base", "registry/consumption_rule"}; }
		ConsumptionRuleRegistry(): UnnamedJSONRegistry(ID()) {}
	};
}
