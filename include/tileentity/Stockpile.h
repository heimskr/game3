#pragma once

#include "tileentity/Chest.h"

namespace Game3 {
	class Stockpile: public Chest {
		public:
			static Identifier ID() { return {"base", "te/stockpile"}; }

			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, ItemStack *, Hand) override;

		protected:
			Stockpile() = default;
			Stockpile(Identifier tilename, const Position &position_);

			friend class TileEntity;
	};
}
