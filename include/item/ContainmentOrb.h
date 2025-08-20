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
			ContainmentOrb(ItemID id, std::string name, MoneyCount basePrice, ItemCount maxCount = 64):
				Item(std::move(id), std::move(name), basePrice, maxCount) {}

			bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
			bool drag(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>, DragAction) override;
			std::string getTooltip(const ConstItemStackPtr &) override;
			Identifier getTextureIdentifier(const ConstItemStackPtr &) const override;
			bool isTextureCacheable() const override { return false; }

			static EntityPtr makeEntity(const ItemStackPtr &);
			/** Returns whether the stack contains a valid (empty or otherwise) containment orb. */
			static bool validate(const ItemStackPtr &);
			static bool isEmpty(const ItemStackPtr &);
			static void saveToJSON(const EntityPtr &, boost::json::value &, bool can_modify = false);

		private:
			/** Returns whether the data was updated. */
			static bool denseClick(const Place &, boost::json::object &, bool release);
			static bool regularClick(const Place &, boost::json::object &);

			static EntityPtr takeEntity(const Place &);
			static bool releaseEntity(const Place &, boost::json::value &);
	};
}
