#pragma once

#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"
#include "ui/module/GTKGeneticAnalysisModule.h"

namespace Game3 {
	class Recombinator: public InventoriedTileEntity, public EnergeticTileEntity {
		public:
			static Identifier ID() { return {"base", "te/recombinator"}; }

			std::string getName() const override { return "Recombinator"; }

			void init(Game &) override;
			void tick(const TickArgs &) override;
			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void absorbJSON(const std::shared_ptr<Game> &, const nlohmann::json &) override;

			bool mayInsertItem(const ItemStackPtr &, Direction, Slot) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast(bool force) override;

			GamePtr getGame() const final;

		private:
			Recombinator();
			Recombinator(Identifier tile_id, Position);
			Recombinator(Position);

			bool combine();

		friend class TileEntity;
	};
}
