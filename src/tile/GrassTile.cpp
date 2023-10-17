#include "Log.h"
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
		auto &realm = *place.realm;

		std::uniform_int_distribution distribution{0, 99};
		if (distribution(threadContext.rng) != 0)
			return;

		const auto [row, column] = place.position;
		auto &tileset = realm.getTileset();

		// Don't overwrite existing tiles.
		if (place.get(Layer::Submerged) != 0)
			return;

		// If there are any adjacent or overlapping flowers, give up and don't spawn anything.
		constexpr Index radius = 3;
		for (Index y = row - radius; y <= row + radius; ++y)
			for (Index x = column - radius; x <= column + radius; ++x)
				if (auto tile_id = realm.tryTile(Layer::Submerged, {y, x}); tile_id && tileset.isInCategory(tileset[*tile_id], "base:category/flowers"))
					return;

		place.set(Layer::Submerged, choose(tileset.getTilesByCategory("base:category/flowers"), threadContext.rng));
	}
}
