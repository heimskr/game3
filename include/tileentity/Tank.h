#pragma once

#include "Texture.h"
#include "game/Inventory.h"
#include "tileentity/FluidHoldingTileEntity.h"

namespace Game3 {
	class Tank: public FluidHoldingTileEntity {
		public:
			static Identifier ID() { return {"base", "te/tank"}; }

			Tank(const Tank &) = delete;
			Tank(Tank &&) = default;
			~Tank() override = default;

			Tank & operator=(const Tank &) = delete;
			Tank & operator=(Tank &&) = default;

			FluidAmount getMaxLevel(FluidID) const override;

			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers) override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			void render(SpriteRenderer &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		private:
			TileID cachedTile = -1;

			Tank() = default;
			Tank(Identifier tile_id, Position);
			Tank(Position);

			friend class TileEntity;

	};
}
