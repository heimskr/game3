#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class CraftingStation: public TileEntity {
		public:
			static Identifier ID() { return {"base", "te/crafting_station"}; }
			Identifier stationType;
			Identifier itemName;

			CraftingStation(const CraftingStation &) = delete;
			CraftingStation(CraftingStation &&) noexcept = default;

			CraftingStation & operator=(const CraftingStation &) = delete;
			CraftingStation & operator=(CraftingStation &&) noexcept = default;

			void toJSON(boost::json::value &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		friend class TileEntity;

		protected:
			CraftingStation() = default;
			CraftingStation(Identifier, const Position &, Identifier station_type, Identifier item_name = {});
	};
}
