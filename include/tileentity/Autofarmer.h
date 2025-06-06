#pragma once

#include "tileentity/DirectedTileEntity.h"
#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class Autofarmer: public InventoriedTileEntity, public EnergeticTileEntity, public DirectedTileEntity {
		public:
			static Identifier ID() { return {"base", "te/autofarmer"}; }

			std::string getName() const override { return "Autofarmer"; }

			void init(Game &) override;
			void tick(const TickArgs &) override;
			void toJSON(boost::json::value &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast(bool force) override;

			/** Returns the number of successful operations that should cause energy to be consumed. */
			size_t autofarm();

		protected:
			std::string getDirectedTileBase() const override { return "base:tile/autofarmer_"; }

		private:
			Position centerOffset{};

			Autofarmer();
			Autofarmer(Identifier tile_id, Position);
			Autofarmer(Position);

			/** Returns whether the operation was successful and energy should be consumed. */
			bool autofarm(Position, bool &input_empty);

		friend class TileEntity;
	};
}
