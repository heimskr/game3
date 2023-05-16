#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class CraftingStation: public TileEntity {
		public:
			static Identifier ID() { return {"base", "te/crafting_station"}; }
			Identifier stationType;

			CraftingStation(const CraftingStation &) = delete;
			CraftingStation(CraftingStation &&) = default;
			~CraftingStation() override = default;

			CraftingStation & operator=(const CraftingStation &) = delete;
			CraftingStation & operator=(CraftingStation &&) = default;

			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &) override;
			void absorbJSON(Game &, const nlohmann::json &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

			friend class TileEntity;

		protected:
			CraftingStation() = default;
			CraftingStation(Identifier, const Position &, Identifier station_type);
	};
}
