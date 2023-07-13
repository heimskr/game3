#pragma once

#include "Directions.h"
#include "Extractors.h"
#include "Types.h"
#include "container/DirectionalContainer.h"
#include "tileentity/TileEntity.h"

namespace Game3 {
	class PipeNetwork;

	class Pipe: public TileEntity {
		friend class PipeLoader;

		protected:
			Directions directions;
			Extractors extractors;
			Identifier corner;
			TileID tileID = 0;
			TileID extractorsCorner = -1;
			std::weak_ptr<PipeNetwork> weakNetwork;
			bool loaded = false;

			Pipe() = default;
			Pipe(Identifier tile_entity_id, Identifier corner_, Position);

			DirectionalContainer<std::shared_ptr<Pipe>> getConnected() const;
			void updateTileID();

		public:

			void render(SpriteRenderer &) override;

			inline auto & getDirections() { return directions; }
			inline const auto & getDirections() const { return directions; }

			inline auto & getExtractors() { return extractors; }
			inline const auto & getExtractors() const { return extractors; }

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

			/** Implicitly marks the pipe as loaded. */
			void setNetwork(const std::shared_ptr<PipeNetwork> &);
			std::shared_ptr<PipeNetwork> getNetwork() const;

			template <typename T>
			inline void toggle(T value) {
				if (!directions.toggle(value))
					extractors[value] = false;
			}
	};
}
