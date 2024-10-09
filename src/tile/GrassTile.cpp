#include "Log.h"
#include "Options.h"
#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/ItemEntity.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "tile/GrassTile.h"
#include "util/Util.h"

namespace Game3 {
	GrassTile::GrassTile():
		Tile(ID()) {}

	void GrassTile::randomTick(const Place &place) {
		Tile::randomTick(place);

		Realm &realm = *place.realm;


		auto [row, column] = place.position;
		const Tileset &tileset = realm.getTileset();

		if constexpr (SPAWN_BIG_FLOWERS) {
			if (threadContext.random(0, 100) != 0) {
				return;
			}

			// Don't overwrite existing tiles.
			if (place.get(Layer::Submerged) != 0)
				return;

			// If there are any adjacent or overlapping flowers, give up and don't spawn anything.
			constexpr Index radius = 3;
			for (Index y = row - radius; y <= row + radius; ++y) {
				for (Index x = column - radius; x <= column + radius; ++x) {
					if (std::optional<TileID> tile_id = realm.tryTile(Layer::Submerged, {y, x}); tile_id && tileset.isInCategory(tileset[*tile_id], "base:category/flowers")) {
						return;
					}
				}
			}

			place.set(Layer::Submerged, choose(tileset.getTilesByCategory("base:category/flowers"), threadContext.rng));
			return;
		}

		if (threadContext.random(0, 50) != 0) {
			return;
		}

		if (std::optional<TileID> tile_id = place.get(Layer::Terrain); tile_id && tileset.isInCategory(*tile_id, "base:category/small_flowers")) {

			constexpr Index spawn_radius = 3;
			row += threadContext.random<Index>(0, spawn_radius + 1);
			column += threadContext.random<Index>(0, spawn_radius + 1);

			Place spawn_place = place.withPosition({row, column});

			if (spawn_place.position == place.position || spawn_place.get(Layer::Terrain) != tileset["base:tile/grass"]) {
				return;
			}

			if (std::hash<Position>{}(spawn_place.position) < std::numeric_limits<Position::IntType>::max() / 2) {
				return;
			}

			spawn_place.set(Layer::Terrain, *tile_id);
		}
	}
}
