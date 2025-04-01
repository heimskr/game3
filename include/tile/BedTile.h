#pragma once

#include "tile/Tile.h"

namespace Game3 {
	class BedTile: public Tile {
		public:
			using Tile::Tile;

			void jumpedFrom(const EntityPtr &entity, const Place &place, Layer) final;
	};
}
