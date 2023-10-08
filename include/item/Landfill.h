#pragma once

#include <functional>
#include <optional>
#include <utility>

#include "Position.h"
#include "item/Item.h"

namespace Game3 {
	class Landfill: public Item {
		public:
			constexpr static ItemCount DEFAULT_COUNT = 1;

			Landfill(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count, Identifier terrain_name, Identifier objects_name = {}, ItemCount required_count = DEFAULT_COUNT);

			bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>) override;
			bool drag(Slot, ItemStack &, const Place &, Modifiers) override;
			bool canUseOnWorld() const override;

		private:
			Identifier terrainName;
			Identifier objectsName;
			ItemCount  requiredCount = -1;

			bool fixRequirement();
	};
}
