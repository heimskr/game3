#include "Log.h"
#include "Position.h"
#include "Tileset.h"
#include "entity/ItemEntity.h"
#include "game/Crop.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "tile/CropTile.h"
#include "util/Util.h"

namespace Game3 {
	CropTile::CropTile(std::shared_ptr<Crop> crop_):
		Tile(ID()), crop(std::move(crop_)) {}

	void CropTile::randomTick(const Place &place) {
		auto &realm = *place.realm;
		auto &tileset = realm.getTileset();

		const auto tile = place.get(Layer::Submerged);
		if (!tile)
			return;

		const Identifier tilename = tileset[*tile];
		if (tilename == crop->stages.back())
			return;

		static std::uniform_real_distribution distribution(0.0, 1.0);
		if (crop->chance < distribution(threadContext.rng))
			return;

		auto iter = std::find(crop->stages.begin(), crop->stages.end(), tilename);
		place.set(Layer::Submerged, *++iter);

		// const auto [row, column] = place.position;
		// auto &tileset = realm.getTileset();

		// If there are any adjacent or overlapping flowers, give up and don't spawn anything.
		// constexpr Index radius = 3;
		// for (Index y = row - radius; y <= row + radius; ++y)
		// 	for (Index x = column - radius; x <= column + radius; ++x)
		// 		if (auto tile_id = realm.tryTile(Layer::Submerged, {y, x}); tile_id && tileset.isInCategory(tileset[*tile_id], "base:category/flowers"))
		// 			return;

		// place.set(Layer::Submerged, choose(tileset.getTilesByCategory("base:category/flowers"), threadContext.rng));
	}
}
