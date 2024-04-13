#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class PressurePlate: public TileEntity {
		public:
			bool down = false;

			static Identifier ID() { return {"base", "te/pressure_plate"}; }

			std::string getName() const override { return "Pressure Plate"; }

			void onOverlap(const EntityPtr &) override;
			void onOverlapEnd(const EntityPtr &) override;

			bool isDown() const;
			void setDown(bool);

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		protected:
			PressurePlate() = default;
			PressurePlate(Identifier tilename, Position);
			PressurePlate(Position);

		friend class TileEntity;
	};
}
