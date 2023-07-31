#pragma once

#include "Texture.h"
#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/FluidHoldingTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class GeothermalGenerator: public InventoriedTileEntity, public FluidHoldingTileEntity, public EnergeticTileEntity {
		public:
			static Identifier ID() { return {"base", "te/geothermal_generator"}; }

			constexpr static EnergyAmount ENERGY_CAPACITY = 64'000;
			constexpr static FluidAmount  FLUID_CAPACITY  = 16 * FluidTile::FULL;
			constexpr static float PERIOD = 0.25;

			bool canInsertItem(const ItemStack &, Direction) override;
			FluidAmount getMaxLevel(FluidID) override;
			EnergyAmount getEnergyCapacity() override;

			void init(Game &) override;
			void tick(Game &, float) override;
			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers) override;
			void absorbJSON(Game &, const nlohmann::json &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast() override;

			Game & getGame() const final;

		private:
			float accumulatedTime = 0.f;
			Lockable<std::optional<std::unordered_set<FluidID>>> supportedFluids;

			GeothermalGenerator();
			GeothermalGenerator(Identifier tile_id, Position);
			GeothermalGenerator(Position);

			friend class TileEntity;
	};
}
