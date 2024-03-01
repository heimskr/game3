#pragma once

#include <functional>
#include <optional>
#include <utility>

#include "types/Position.h"
#include "item/Item.h"

namespace Game3 {
	class Mushroom: public Item {
		public:
			using ID = uint32_t;

			ID subID;

			Mushroom(ItemID id_, std::string name_, MoneyCount base_price, ID sub_id);

			void getOffsets(const Game &, std::shared_ptr<Texture> &, float &x_offset, float &y_offset) override;
			Glib::RefPtr<Gdk::Pixbuf> makeImage(const Game &, const ConstItemStackPtr &) const override;
	};
}
