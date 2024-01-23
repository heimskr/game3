#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class Stockpile: public TileEntity {
		public:
			static Identifier ID() { return {"base", "te/stockpile"}; }

			std::string getName() const override { return "Stockpile"; }
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, ItemStack *, Hand) override;

		protected:
			VillageID villageID{};

			Stockpile() = default;
			Stockpile(Identifier tilename, const Position &position_, VillageID);

			friend class TileEntity;
	};
}
