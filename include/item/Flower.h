#pragma once

#include "item/Plantable.h"

namespace Game3 {
	class Flower: public Plantable {
		public:
			Identifier tilename;
			Identifier smallTilename;
			/** Can be empty to indicate that the flower can be planted on any walkable tile. */
			Identifier validGround;

			Flower(ItemID, std::string name, Identifier tilename, Identifier smallTilename, Identifier validGround, MoneyCount basePrice, ItemCount maxCount = 64);

			bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
			bool plant(InventoryPtr, Slot, const ItemStackPtr &, const Place &, Layer) override;

		private:
			bool plantTile(InventoryPtr, Slot, const ItemStackPtr &, const Place &, Layer, const Identifier &);
	};
}
