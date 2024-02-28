#pragma once

#include "item/Item.h"

namespace Game3 {
	struct Copier: Item {
		using Item::Item;
		bool drag(Slot, ItemStack &, const Place &, Modifiers) override;
		void renderEffects(const RendererContext &, ItemStack &) const;
	};
}
