#pragma once

#include "item/Item.h"

#include <memory>

namespace Game3 {
	class Realm;

	struct Copier: Item {
		using Item::Item;
		bool use(Slot, ItemStack &, const std::shared_ptr<Player> &, Modifiers) override;
		bool drag(Slot, ItemStack &, const Place &, Modifiers) override;
		void renderEffects(const RendererContext &, ItemStack &) const;

		std::string getTiles(const ItemStack &, const std::shared_ptr<Realm> &) const;
	};
}
