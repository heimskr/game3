#pragma once

#include "tileentity/TileEntity.h"
#include "types/Direction.h"

namespace Game3 {
	class DirectedTileEntity: public virtual TileEntity {
		public:
			inline Direction getDirection() const { return tileDirection; }
			void setDirection(Direction);
			void rotateClockwise();
			void rotateCounterClockwise();

			void toJSON(boost::json::value &) const override;
			void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &) override;
			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		protected:
			Direction tileDirection;

			DirectedTileEntity(Direction = Direction::Down);

			virtual std::string getDirectedTileBase() const = 0;
	};
}
