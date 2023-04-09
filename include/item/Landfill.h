#pragma once

#include <functional>
#include <optional>
#include <utility>

#include "Position.h"
#include "item/Item.h"

namespace Game3 {
	class Landfill: public Item {
		public:
			constexpr static ItemCount DEFAULT_COUNT = 2;

			struct Result {
				ItemStack required;
				Identifier newTile;
			};

			using RequirementFn = std::function<std::optional<Result>(const Place &)>;

			std::optional<RequirementFn> requirement;

			Landfill(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count, RequirementFn);

			// TODO: support tilemap predicates (in case other realms end up not using the monomap)
			Landfill(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count, Identifier tileset_name, Identifier required_tile, ItemStack requirement, Identifier new_tile);

			/** Like the other function-creating instruction, but using this item as a requirement */
			Landfill(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count, Identifier tileset_name, Identifier required_tile, ItemCount required_count, Identifier new_tile);

			bool use(Slot, ItemStack &, const Place &) override;

			virtual std::optional<Result> callRequirement(const Place &);

			bool canUseOnWorld() const override { return true; }

		private:
			Identifier tilesetName;
			Identifier requiredTile;
			ItemCount requiredCount = -1;
			Identifier newTile;

			bool fixRequirement();
	};
}
