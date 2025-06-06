#pragma once

#include <functional>
#include <optional>
#include <utility>

#include "types/Position.h"
#include "item/Item.h"

namespace Game3 {
	class Landfill: public Item {
		public:
			constexpr static ItemCount DEFAULT_COUNT = 1;

			Landfill(ItemID id, std::string name, MoneyCount basePrice, ItemCount maxCount, Layer terrainLayer, Identifier terrainName, Identifier objectsName = {}, ItemCount requiredCount = DEFAULT_COUNT);

			bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
			bool drag(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>, DragAction) override;
			bool canUseOnWorld() const override;

		private:
			Identifier terrainName;
			Identifier objectsName;
			ItemCount requiredCount = -1;
			Layer terrainLayer;

			bool fixRequirement();
	};
}
