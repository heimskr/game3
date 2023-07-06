#pragma once

#include "Directions.h"
#include "Extractors.h"
#include "Types.h"
#include "container/DirectionalContainer.h"
#include "tileentity/TileEntity.h"

namespace Game3 {
	class Pipe: public TileEntity {
		protected:
			Directions directions;
			Extractors extractors;
			Identifier corner;
			TileID tileID = 0;
			TileID extractorsCorner = -1;

			Pipe() = default;
			Pipe(Identifier tile_entity_id, Identifier corner_, Position);

			DirectionalContainer<std::shared_ptr<Pipe>> getConnected() const;
			void updateTileID();

		public:
			void render(SpriteRenderer &) override;

			inline auto & getExtractors() { return extractors; }
			inline const auto & getExtractors() const { return extractors; }

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

			template <typename T>
			inline void toggle(T value) { directions.toggle(value); }
	};
}
