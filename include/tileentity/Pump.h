#pragma once

#include "Texture.h"
#include "game/Inventory.h"
#include "tileentity/FluidHoldingTileEntity.h"

namespace Game3 {
	class Pump: public FluidHoldingTileEntity {
		public:
			static Identifier ID() { return {"base", "te/pump"}; }

			Pump(const Pump &) = delete;
			Pump(Pump &&) = default;
			~Pump() override = default;

			Pump & operator=(const Pump &) = delete;
			Pump & operator=(Pump &&) = default;

			inline Direction getDirection() const { return pumpDirection; }
			void setDirection(Direction);

			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers) override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			void render(SpriteRenderer &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;


		protected:
			Direction pumpDirection = Direction::Down;

			Pump() = default;
			Pump(Identifier tile_id, Position);
			Pump(Position);

			friend class TileEntity;

		private:
			TileID cachedTile = -1;
	};
}
