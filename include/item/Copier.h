#pragma once

#include "item/Item.h"

namespace Game3 {
	struct Copier: Item {
		using Item::Item;
		bool use(Slot, ItemStack &, const std::shared_ptr<Player> &, Modifiers) override;
		bool drag(Slot, ItemStack &, const Place &, Modifiers) override;
		void renderEffects(const RendererContext &, ItemStack &) const;
	};
}
