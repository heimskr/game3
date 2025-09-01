#pragma once

#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class Sequencer: public InventoriedTileEntity, public EnergeticTileEntity {
		public:
			static Identifier ID() { return {"base", "te/sequencer"}; }

			std::string getName() const override { return "Sequencer"; }

			void init(Game &) override;
			void tick(const TickArgs &) override;
			void toJSON(boost::json::value &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &) override;

			bool mayInsertItem(const ItemStackPtr &, Direction, Slot) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, BasicBuffer &) override;
			void broadcast(bool force) override;

			GamePtr getGame() const final;

		private:
			Sequencer();
			Sequencer(Identifier tile_id, Position);
			Sequencer(Position);

		friend class TileEntity;
	};
}
