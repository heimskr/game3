#pragma once

#include "item/Item.h"
#include "threading/Lockable.h"

#include <unordered_map>

namespace Game3 {
	class ChemicalItem: public Item {
		public:
			static Identifier ID() { return {"base", "item/chemical"}; }

			using Item::Item;

			TexturePtr getTexture(const Game &, const ConstItemStackPtr &) const override;
			TexturePtr makeTexture(const Game &, const ConstItemStackPtr &) const override;

			std::string getTooltip(const ConstItemStackPtr &) override;

		private:
			static Lockable<std::unordered_map<std::string, TexturePtr>> textureCache;

			static std::string getFormula(const ItemStack &);
	};
}
