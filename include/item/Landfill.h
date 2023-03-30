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
				TileID newTile;
			};

			using RequirementFn = std::function<std::optional<Result>(const Place &)>;

			std::optional<RequirementFn> requirement;

			Landfill(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count, RequirementFn);

			// TODO: support tilemap predicates (in case other realms end up not using the monomap)
			Landfill(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count, TileID required_tile, ItemStack requirement, TileID new_tile);

			/** Like the other function-creating instruction, but using this item as a requirement */
			Landfill(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count, TileID required_tile, ItemCount required_count, TileID new_tile);

			bool use(Slot, ItemStack &, const Place &) override;

			virtual std::optional<Result> callRequirement(const Place &);

			bool canUseOnWorld() const override { return true; }

		private:
			TileID requiredTile = -1;
			ItemCount requiredCount = -1;
			TileID newTile = -1;

			bool fixRequirement();
	};
}
