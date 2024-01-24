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

	class ProductionRule: public Registerable {
		public:
			using Registerable::Registerable;
			ProductionRule(const Game &, const nlohmann::json &);

			inline const auto & getInputs() const { return inputs; }
			inline const auto & getOutput() const { return output; }
			inline auto getCap()   const { return cap;   }
			inline auto getLabor() const { return labor; }
			inline const auto & getBiomes()      const { return biomes;      }
			inline const auto & getRandomRange() const { return randomRange; }

			static ProductionRule fromJSON(const Game &, const nlohmann::json &);

		private:
			std::vector<ItemStack> inputs;
			ItemStack output;
			LaborAmount labor = 0;
			std::optional<double> cap;
			std::optional<std::set<BiomeType>> biomes;
			std::optional<std::pair<double, double>> randomRange;

		friend void to_json(nlohmann::json &, const ProductionRule &);
	};

	void to_json(nlohmann::json &, const ProductionRule &);

	struct ProductionRuleRegistry: UnnamedJSONRegistry<ProductionRule> {
		static Identifier ID() { return {"base", "registry/production_rule"}; }
		ProductionRuleRegistry(): UnnamedJSONRegistry(ID()) {}
	};
}
