#pragma once

#include "Texture.h"
#include "game/Inventory.h"
#include "tileentity/FluidHoldingTileEntity.h"

namespace Game3 {
	class Pump: public FluidHoldingTileEntity {
		public:
			static Identifier ID() { return {"base", "te/pump"}; }

			constexpr static float PERIOD = 0.25;

			FluidAmount extractionRate = 250;

			Pump(const Pump &) = delete;
			Pump(Pump &&) = default;
			~Pump() override = default;

			Pump & operator=(const Pump &) = delete;
			Pump & operator=(Pump &&) = default;

			inline Direction getDirection() const { return pumpDirection; }
			void setDirection(Direction);

			FluidAmount getMaxLevel(FluidID) const override;

			void tick(Game &, float) override;
			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers) override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			void render(SpriteRenderer &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		private:
			TileID cachedTile = -1;
			float accumulatedTime = 0.f;
			Direction pumpDirection = Direction::Down;

			Pump() = default;
			Pump(Identifier tile_id, Position);
			Pump(Position);

			friend class TileEntity;

	};
}
