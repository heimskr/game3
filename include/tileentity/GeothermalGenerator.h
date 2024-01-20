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

			bool mayInsertItem(const ItemStack &, Direction, Slot) override;
			bool mayExtractItem(Direction, Slot) override;
			bool canInsertItem(const ItemStack &, Direction, Slot) override;
			FluidAmount getMaxLevel(FluidID) override;
			EnergyAmount getEnergyCapacity() override;

			std::string getName() const override { return "Geothermal Generator"; }

			void init(Game &) override;
			void tick(Game &, float) override;
			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, ItemStack *, Hand) override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			bool populateMenu(const PlayerPtr &, bool overlap, const std::string &id, Glib::RefPtr<Gio::Menu>, Glib::RefPtr<Gio::SimpleActionGroup>) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast(bool force) override;

			Game & getGame() const final;

		private:
			Lockable<std::optional<std::unordered_set<FluidID>>> supportedFluids;

			GeothermalGenerator();
			GeothermalGenerator(Identifier tile_id, Position);
			GeothermalGenerator(Position);

			void slurpFlasks();

			friend class TileEntity;
	};
}
