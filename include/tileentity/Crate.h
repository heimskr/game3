#pragma once

#include "data/Identifier.h"
#include "game/Inventory.h"
#include "threading/Lockable.h"
#include "tileentity/InventoriedTileEntity.h"
#include "types/Types.h"

#include <string>

namespace Game3 {
	class Crate: public InventoriedTileEntity {
		public:
			static Identifier ID() { return {"base", "te/crate"}; }

			~Crate() override = default;

			Identifier itemName;
			std::string name;

			std::string getName() const override;

			ItemCount itemsInsertable(const ItemStackPtr &, Direction, Slot) override;

			void toJSON(boost::json::value &) const override;
			bool onInteractNextTo(const PlayerPtr &, Modifiers, const ItemStackPtr &, Hand) override;
			void absorbJSON(const GamePtr &, const boost::json::value &) override;

			void setInventory(Slot slot_count) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		protected:
			Crate() = default;
			Crate(Identifier tile_id, const Position &, Identifier item_name, std::string name_);
			Crate(const Position &);

		friend class TileEntity;
	};
}
