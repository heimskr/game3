#include "Log.h"
#include "Position.h"
#include "Tileset.h"
#include "entity/Player.h"
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
		assert(!crop->stages.empty());
		if (tilename == crop->stages.back())
			return;

		static std::uniform_real_distribution distribution(0.0, 1.0);
		if (crop->chance < distribution(threadContext.rng))
			return;

		auto iter = std::find(crop->stages.begin(), crop->stages.end(), tilename);
		place.set(Layer::Submerged, *++iter);
	}

	bool CropTile::interact(const Place &place, Layer layer) {
		assert(!crop->stages.empty());

		if (auto tilename = place.getName(layer); tilename && *tilename == crop->stages.back()) {
			place.set(layer, 0);
			place.player->give(crop->product);
			return true;
		}

		return false;
	}
}
