#pragma once

#include "container/RandomSet.h"
#include "item/Item.h"
#include "registry/Registry.h"
#include "types/Types.h"

#include <boost/json/fwd.hpp>

#include <optional>
#include <set>
#include <utility>
#include <vector>

namespace Game3 {
	class Game;

	class ProductionRule: public Registerable {
		public:
			using Registerable::Registerable;
			ProductionRule(const std::shared_ptr<Game> &, const boost::json::value &);

			inline const auto & getInputs() const { return inputs; }
			inline const auto & getOutput() const { return output; }
			inline auto getCap()   const { return cap;   }
			inline auto getLabor() const { return labor; }
			inline auto getRichnessEffect() const { return richnessEffect;   }
			inline const auto & getBiomes() const { return biomes;           }
			inline const auto & getRandomRange() const { return randomRange; }

			bool doesBiomeMatch(BiomeType) const;


		private:
			std::vector<ItemStackPtr> inputs;
			ItemStackPtr output;
			LaborAmount labor = 0;
			/** If present, the richness of the resource is multiplied by this and then used as a multiplier for the production rule. */
			std::optional<double> richnessEffect;
			std::optional<double> cap;
			std::optional<std::set<BiomeType>> biomes;
			std::optional<std::pair<double, double>> randomRange;
	};

	ProductionRule tag_invoke(boost::json::value_to_tag<ProductionRule>, const boost::json::value &, const std::shared_ptr<Game> &);
	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const ProductionRule &);

	struct ProductionRuleRegistry: UnnamedJSONRegistry<ProductionRule, RandomSet> {
		static Identifier ID() { return {"base", "registry/production_rule"}; }
		ProductionRuleRegistry(): UnnamedJSONRegistry(ID()) {}
	};
}
