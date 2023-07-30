#pragma once

#include "Texture.h"
#include "tileentity/EnergeticTileEntity.h"

namespace Game3 {
	class GeothermalGenerator: public EnergeticTileEntity {
		public:
			static Identifier ID() { return {"base", "te/geothermal_generator"}; }

			EnergyAmount getEnergyCapacity() override;

			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers) override;
			void absorbJSON(Game &, const nlohmann::json &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		private:
			GeothermalGenerator() = default;
			GeothermalGenerator(Identifier tile_id, Position);
			GeothermalGenerator(Position);

			friend class TileEntity;
	};
}
