#pragma once

#include "game/Inventory.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class Chest: public InventoriedTileEntity {
		public:
			static Identifier ID() { return {"base", "te/chest"}; }

			std::string name;
			Identifier itemName{"base", "item/chest"};

			std::string getName() const override;

			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, ItemStack *, Hand) override;
			void absorbJSON(Game &, const nlohmann::json &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		protected:
			Chest() = default;
			Chest(Identifier tile_id, const Position &, std::string name_, Identifier item_name = {"base", "item/chest"});
			Chest(const Position &);

			friend class TileEntity;
	};
}
