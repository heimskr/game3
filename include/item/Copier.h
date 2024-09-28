#pragma once

#include "item/Item.h"

#include <memory>
#include <optional>

namespace Game3 {
	class Realm;

	class Copier: public Item {
		public:
			using Item::Item;
			bool use(Slot, const ItemStackPtr &, const std::shared_ptr<Player> &, Modifiers) override;
			bool drag(Slot, const ItemStackPtr &, const Place &, Modifiers) override;
			void renderEffects(const RendererContext &, const Position &, Modifiers, const ItemStackPtr &) const override;
			// bool populateMenu(const InventoryPtr &, Slot, const ItemStackPtr &, Glib::RefPtr<Gio::Menu>, Glib::RefPtr<Gio::SimpleActionGroup>) const override;

			static std::string getString(const ItemStackPtr &, const std::shared_ptr<Realm> &);

		private:
			std::optional<Position> computeMinimums(const std::unordered_set<Position> &);
	};
}
