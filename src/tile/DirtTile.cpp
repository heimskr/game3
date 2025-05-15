#include "util/Log.h"
#include "types/Position.h"
#include "graphics/Tileset.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "tile/DirtTile.h"
#include "util/Util.h"

namespace Game3 {
	DirtTile::DirtTile():
		InfiniteShovelableTile(ID(), "base:item/dirt") {}

	void DirtTile::randomTick(const Place &place) {
		Tile::randomTick(place);

		std::uniform_int_distribution distribution{1, 100};
		if (distribution(threadContext.rng) <= 50)
			return;

		Realm &realm = *place.realm;
		Tileset &tileset = realm.getTileset();

		for (const Direction direction: ALL_DIRECTIONS) {
			if (auto tile = realm.tryTile(Layer::Terrain, place.position + direction); tile && tileset.isInCategory(*tile, "base:category/spreads_on_dirt")) {
				place.set(Layer::Terrain, *tile);
				break;
			}
		}
	}
}
