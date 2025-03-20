#pragma once

#include "data/Identifier.h"
#include "fluid/Fluid.h"
#include "item/Item.h"
#include "types/Types.h"
#include "ui/Modifiers.h"

#include <string>
#include <utility>

namespace Game3 {
	class ContainmentOrb: public Item {
		public:
			ContainmentOrb(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count = 64):
				Item(std::move(id_), std::move(name_), base_price, max_count) {}

			bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
			std::string getTooltip(const ConstItemStackPtr &) override;
			Identifier getTextureIdentifier(const ConstItemStackPtr &) const override;
			bool isTextureCacheable() const override { return false; }

			static EntityPtr makeEntity(const ItemStackPtr &);
			/** Returns whether the stack contains a valid (empty or otherwise) containment orb. */
			static bool validate(const ItemStackPtr &);
			static bool isEmpty(const ItemStackPtr &);
			static void saveToJSON(const EntityPtr &, boost::json::value &, bool can_modify = false);
	};
}
