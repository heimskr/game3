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

			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &) override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			void render(SpriteRenderer &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		protected:
			Pump() = default;
			Pump(Identifier tile_id, Position);
			Pump(Position);

			friend class TileEntity;

		private:
			TileID cachedTile = -1;
	};
}
