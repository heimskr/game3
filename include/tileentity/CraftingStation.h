#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class CraftingStation: public TileEntity {
		public:
			static Identifier ID() { return {"base", "te/crafting_station"}; }
			Identifier stationType;
			Identifier itemName;

			CraftingStation(const CraftingStation &) = delete;
			CraftingStation(CraftingStation &&) = default;
			~CraftingStation() override = default;

			CraftingStation & operator=(const CraftingStation &) = delete;
			CraftingStation & operator=(CraftingStation &&) = default;

			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, ItemStack *, Hand) override;
			void absorbJSON(const std::shared_ptr<Game> &, const nlohmann::json &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

			friend class TileEntity;

		protected:
			CraftingStation() = default;
			CraftingStation(Identifier, const Position &, Identifier station_type, Identifier item_name = {});
	};
}
