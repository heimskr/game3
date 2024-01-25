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
			inline auto getRichnessEffect() const { return richnessEffect;   }
			inline const auto & getBiomes() const { return biomes;           }
			inline const auto & getRandomRange() const { return randomRange; }

			bool doesBiomeMatch(BiomeType) const;

			static ProductionRule fromJSON(const Game &, const nlohmann::json &);

		private:
			std::vector<ItemStack> inputs;
			ItemStack output;
			LaborAmount labor = 0;
			/** If present, the richness of the resource is multiplied by this and then used as a multiplier for the production rule. */
			std::optional<double> richnessEffect;
			std::optional<double> cap;
			std::optional<std::set<BiomeType>> biomes;
			std::optional<std::pair<double, double>> randomRange;
	};

	void to_json(nlohmann::json &, const ProductionRule &);

	struct ProductionRuleRegistry: UnnamedJSONRegistry<ProductionRule> {
		static Identifier ID() { return {"base", "registry/production_rule"}; }
		ProductionRuleRegistry(): UnnamedJSONRegistry(ID()) {}
	};
}
