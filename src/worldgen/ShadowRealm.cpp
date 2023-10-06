#include "graphics/Tileset.h"
#include "biome/Biome.h"
#include "game/Game.h"
#include "realm/ShadowRealm.h"
#include "realm/Realm.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/ShadowRealm.h"
#include "worldgen/WorldGen.h"

#include <thread>

namespace Game3::WorldGen {
	void generateShadowRealm(const std::shared_ptr<Realm> &realm, size_t, const WorldGenParams &params, const ChunkRange &range, bool initial_generation) {
		realm->markGenerated(range);
		Timer shadow_realm_timer("GenShadowRealm");

		auto guard = realm->guardGeneration();

		TileProvider &provider = realm->tileProvider;

		for (auto y = range.topLeft.y; y <= range.bottomRight.y; ++y)
			for (auto x = range.topLeft.x; x <= range.bottomRight.x; ++x)
				provider.ensureAllChunks(ChunkPosition{x, y});

		for (Index row = range.rowMin(); row <= range.rowMax(); ++row) {
			for (Index column = range.columnMin(); column <= range.columnMax(); ++column) {
				std::unique_lock<std::shared_mutex> lock;
				provider.findBiomeType(Position(row, column), &lock) = Biome::VOID;
			}
		}

		range.iterate([&](ChunkPosition chunk_position) {
			chunk_position.iterate([&](Position position) {
				realm->setTile(Layer::Terrain, position, "base:tile/void", false);
			});
		});

		if (initial_generation) {
			const Position center{CHUNK_SIZE / 2, CHUNK_SIZE / 2};
			realm->setTile(Layer::Submerged, center + Position{-2, -3}, "base:tile/guy_w", false);
			realm->setTile(Layer::Submerged, center + Position{-2, -2}, "base:tile/guy_e", false);
			realm->setTile(Layer::Submerged, center + Position{ 0, -2}, "base:tile/candle", false);
			realm->setTile(Layer::Submerged, center + Position{-2,  0}, "base:tile/candle", false);
			realm->setTile(Layer::Submerged, center + Position{ 2,  0}, "base:tile/candle", false);
			realm->setTile(Layer::Submerged, center + Position{ 0,  2}, "base:tile/candle", false);
			realm->setTile(Layer::Submerged, center + Position{-1, -1}, "base:tile/circle_nw", false);
			realm->setTile(Layer::Submerged, center + Position{-1,  0}, "base:tile/circle_n",  false);
			realm->setTile(Layer::Submerged, center + Position{-1,  1}, "base:tile/circle_ne", false);
			realm->setTile(Layer::Submerged, center + Position{ 0, -1}, "base:tile/circle_w",  false);
			realm->setTile(Layer::Submerged, center + Position{ 0,  0}, "base:tile/circle_m",  false);
			realm->setTile(Layer::Submerged, center + Position{ 0,  1}, "base:tile/circle_e",  false);
			realm->setTile(Layer::Submerged, center + Position{ 1, -1}, "base:tile/circle_sw", false);
			realm->setTile(Layer::Submerged, center + Position{ 1,  0}, "base:tile/circle_s",  false);
			realm->setTile(Layer::Submerged, center + Position{ 1,  1}, "base:tile/circle_se", false);
		}

		range.iterate([&](ChunkPosition chunk_position) {
			provider.updateChunk(chunk_position);
		});

		if (initial_generation)
			std::dynamic_pointer_cast<ShadowRealm>(realm)->worldgenParams = params;

		{
			Timer pathmap_timer("RemakePathMap");
			realm->remakePathMap(range);
		}

		shadow_realm_timer.stop();
		if (initial_generation)
			Timer::summary();
		Timer::clear();
	}
}
