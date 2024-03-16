#pragma once

#include "graphics/Texture.h"
#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/FluidHoldingTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class GeothermalGenerator: public InventoriedTileEntity, public FluidHoldingTileEntity, public EnergeticTileEntity {
		public:
			static Identifier ID() { return {"base", "te/geothermal_generator"}; }

			constexpr static EnergyAmount ENERGY_CAPACITY = 64'000;
			constexpr static FluidAmount  FLUID_CAPACITY  = 16 * FluidTile::FULL;

			bool mayInsertItem(const ItemStackPtr &, Direction, Slot) override;
			bool mayExtractItem(Direction, Slot) override;
			bool canInsertItem(const ItemStackPtr &, Direction, Slot) override;
			FluidAmount getMaxLevel(FluidID) override;
			EnergyAmount getEnergyCapacity() override;

			std::string getName() const override { return "Geothermal Generator"; }

			void init(Game &) override;
			void tick(const TickArgs &) override;
			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void absorbJSON(const std::shared_ptr<Game> &, const nlohmann::json &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast(bool force) override;

			GamePtr getGame() const final;

		private:
			Lockable<std::optional<std::unordered_set<FluidID>>> supportedFluids;

			GeothermalGenerator();
			GeothermalGenerator(Identifier tile_id, Position);
			GeothermalGenerator(Position);

			void slurpFlasks();

			friend class TileEntity;
	};
}
