#pragma once

#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class Microscope: public InventoriedTileEntity {
		public:
			static Identifier ID() { return {"base", "te/microscope"}; }

			std::string getName() const override { return "Microscope"; }

			void init(Game &) override;
			void toJSON(boost::json::value &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, BasicBuffer &) override;
			void broadcast(bool force) override;

			GamePtr getGame() const final;

		private:
			Microscope() = default;
			Microscope(Identifier tile_id, Position);
			Microscope(Position);

		friend class TileEntity;
	};
}
