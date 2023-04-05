#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class CraftingStation: public TileEntity {
		public:
			CraftingStationType stationType = CraftingStationType::None;

			CraftingStation(const CraftingStation &) = delete;
			CraftingStation(CraftingStation &&) = default;
			~CraftingStation() override = default;

			CraftingStation & operator=(const CraftingStation &) = delete;
			CraftingStation & operator=(CraftingStation &&) = default;

			TileEntityID getID() const override { return TileEntity::CRAFTING_STATION; }

			void init() override {}
			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &) override;
			void absorbJSON(const nlohmann::json &) override;

			friend class TileEntity;

		protected:
			CraftingStation() = default;
			CraftingStation(TileID, const Position &, CraftingStationType);
	};
}
