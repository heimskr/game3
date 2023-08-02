#pragma once

#include "item/Item.h"
#include "threading/Lockable.h"

#include <unordered_map>

namespace Game3 {
	class ChemicalItem: public Item {
		public:
			static Identifier ID() { return {"base", "item/chemical"}; }

			using Item::Item;

			Glib::RefPtr<Gdk::Pixbuf> getImage(const Game &, const ItemStack &) override;
			Glib::RefPtr<Gdk::Pixbuf> makeImage(const Game &, const ItemStack &) override;

			static Lockable<std::unordered_map<std::string, Glib::RefPtr<Gdk::Pixbuf>>> imageCache;

		private:
			static std::string getFormula(const ItemStack &);
	};
}
