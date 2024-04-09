#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class PressurePlate: public TileEntity {
		public:
			static Identifier ID() { return {"base", "te/pressure_plate"}; }

			std::string getName() const override { return "Pressure Plate"; }

			void onOverlap(const std::shared_ptr<Entity> &) override;

			bool isDown() const;

		protected:
			PressurePlate() = default;
			PressurePlate(Identifier tilename, Position);
			PressurePlate(Position);

		friend class TileEntity;
	};
}
