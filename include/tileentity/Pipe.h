#pragma once

#include "Directions.h"
#include "Types.h"
#include "container/DirectionalContainer.h"
#include "tileentity/TileEntity.h"

namespace Game3 {
	class Pipe: public TileEntity {
		protected:
			Directions directions;
			const Identifier corner;
			TileID tileID = 0;

			Pipe() = default;
			Pipe(Identifier tile_entity_id, Identifier corner_, Position);

			DirectionalContainer<std::shared_ptr<Pipe>> getConnected() const;
			void updateTileID();

		public:
			void render(SpriteRenderer &) override;

			template <typename T>
			inline void toggle(T value) { directions.toggle(value); }
	};
}
