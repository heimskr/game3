#pragma once

#include "item/Item.h"
#include "registry/Registry.h"
#include "types/Types.h"

#include <nlohmann/json_fwd.hpp>

#include <optional>
#include <set>
#include <utility>
#include <vector>

namespace Game3 {
	class Game;

	class ConsumptionRule: public Registerable {
		public:
			using Registerable::Registerable;
			ConsumptionRule(const Game &, const nlohmann::json &);

			inline const auto & getInput() const { return input; }
			inline auto getLaborOut() const { return laborOut; }
			inline auto getAlways() const { return always; }

			static ConsumptionRule fromJSON(const Game &, const nlohmann::json &);

		private:
			Identifier input;
			LaborAmount laborOut{};
			bool always{};
	};

	void to_json(nlohmann::json &, const ConsumptionRule &);

	struct ConsumptionRuleRegistry: UnnamedJSONRegistry<ConsumptionRule> {
		static Identifier ID() { return {"base", "registry/consumption_rule"}; }
		ConsumptionRuleRegistry(): UnnamedJSONRegistry(ID()) {}
	};
}
