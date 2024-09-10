#pragma once

#include "tileentity/FluidHoldingTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class Gene;

	class Mutator: public FluidHoldingTileEntity, public InventoriedTileEntity {
		public:
			static Identifier ID() { return {"base", "te/mutator"}; }

			void mutate(float strength);
			std::unique_ptr<Gene> getGene() const;

			std::string getName() const override { return "Mutator"; }
			void handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) override;

			void init(Game &) override;
			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void absorbJSON(const std::shared_ptr<Game> &, const nlohmann::json &) override;

			FluidAmount getMaxLevel(FluidID) override;
			bool canInsertFluid(FluidStack, Direction) override;
			bool mayInsertItem(const ItemStackPtr &, Direction, Slot) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast(bool force) override;

			GamePtr getGame() const final;

		private:
			std::optional<FluidID> mutagenID;

			Mutator() = default;
			Mutator(Identifier tile_id, Position);
			Mutator(Position);

			void findMutagen();

		friend class GTKMutatorModule;
		friend class TileEntity;
	};
}
