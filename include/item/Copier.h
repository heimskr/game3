#pragma once

#include "item/Item.h"

#include <memory>
#include <optional>

namespace Game3 {
	class Realm;

	class Copier: public Item {
		public:
			using Item::Item;
			bool use(Slot, ItemStack &, const std::shared_ptr<Player> &, Modifiers) override;
			bool drag(Slot, const ItemStackPtr &, const Place &, Modifiers) override;
			void renderEffects(const RendererContext &, const Position &, Modifiers, ItemStack &) const override;
			bool populateMenu(ItemStack &, Glib::RefPtr<Gio::Menu>, Glib::RefPtr<Gio::SimpleActionGroup>) const override;

			std::string getTiles(const ItemStack &, const std::shared_ptr<Realm> &) const;

		private:
			std::optional<Position> computeMinimums(const std::unordered_set<Position> &);
	};
}
