#pragma once

#include "item/Item.h"

namespace Game3 {
	class TerrainSeed: public Item {
		public:
			Identifier targetTilename;
			Identifier replacementTilename;
			Layer targetLayer;
			Layer replacementLayer;

			TerrainSeed(ItemID id, std::string name, Layer targetLayer, Identifier targetTilename, Layer replacementLayer, Identifier replacementTilename, MoneyCount basePrice, ItemCount maxCount = 64);

			bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
			bool drag(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>, DragAction) override;
	};
}
