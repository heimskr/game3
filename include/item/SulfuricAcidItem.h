#pragma once

#include "interface/HasFluidType.h"
#include "item/Item.h"

namespace Game3 {
	class SulfuricAcidItem: public Item, public HasFluidType {
		public:
			using Item::Item;
			Identifier getFluidType() const final;
	};
}
