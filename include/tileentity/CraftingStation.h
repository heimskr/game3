#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class CraftingStation: public TileEntity {
		public:
			Identifier stationType;

			CraftingStation(const CraftingStation &) = delete;
			CraftingStation(CraftingStation &&) = default;
			~CraftingStation() override = default;

			CraftingStation & operator=(const CraftingStation &) = delete;
			CraftingStation & operator=(CraftingStation &&) = default;

			void init() override {}
			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &) override;
			void absorbJSON(const Game &, const nlohmann::json &) override;

			friend class TileEntity;

		protected:
			CraftingStation() = default;
			CraftingStation(TileID, const Position &, Identifier);
	};
}
