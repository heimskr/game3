#include "graphics/Tileset.h"
#include "biome/Biome.h"
#include "game/Game.h"
#include "realm/ShipRealm.h"
#include "realm/Realm.h"
#include "tools/Paster.h"
#include "util/Cast.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/ShipRealmGen.h"
#include "worldgen/WorldGen.h"

#include <thread>

namespace Game3::WorldGen {
	void generateShipRealmChunks(const std::shared_ptr<Realm> &realm, size_t, const WorldGenParams &params, const ChunkRange &range, bool initial_generation) {
		realm->markGenerated(range);
		Timer ship_realm_timer("GenShipRealm");

		auto guard = realm->guardGeneration();

		TileProvider &provider = realm->tileProvider;

		for (auto y = range.topLeft.y; y <= range.bottomRight.y; ++y)
			for (auto x = range.topLeft.x; x <= range.bottomRight.x; ++x)
				provider.ensureAllChunks(ChunkPosition{x, y});

		for (Index row = range.rowMin(); row <= range.rowMax(); ++row) {
			for (Index column = range.columnMin(); column <= range.columnMax(); ++column) {
				std::unique_lock<std::shared_mutex> lock;
				provider.findBiomeType(Position(row, column), &lock) = Biome::SHIP;
			}
		}

		Paster paster;

		range.iterate([&](ChunkPosition chunk_position) {
			chunk_position.iterate([&](const Position &position) {
				realm->setTile(Layer::Terrain, position, "base:tile/sand", false);
				realm->setFluid(position, "base:fluid/water", FluidTile::FULL, true);
			});

			Position top_left = chunk_position.topLeft();
			realm->setTile(Layer::Objects, top_left, "base:tile/barrier", false);

			for (Index i = 1; i < CHUNK_SIZE; ++i) {
				realm->setTile(Layer::Objects, top_left + Position{0, i}, "base:tile/barrier", false);
				realm->setTile(Layer::Objects, top_left + Position{i, 0}, "base:tile/barrier", false);
			}

			if (chunk_position == ChunkPosition{0, 0}) {
				paster.ingest("empty;planks;tall_wooden_wall;sand;wooden_wall;bed2;grate_nw;grate_ne;ship_wheel;grate_mw;grate_me;barrier;grate_sw;grate_se/0,0=1:0:2,,3:0:2,,,,,1:0:2,1:0:4,,,,,,,,,,,,,/1,0=1:0:2,1:0:5,,,,,,1:0:2,1,,,,,,,,,,,,,1:0:4,/2,0=1:0:2,1,,,,,,1:0:2,1,,,,,,,,,1:6,1:7,1,,,1:0:4,/3,0=1:0:2,1,,,,,,,,,1:0:8,1,,,,,,1:9,1:10,1,,,,1:0:4,/4,0=1:0:2,1,,,,,,1:11,1,,,,,,,,,1:12,1:13,1,,,1:0:4,/5,0=1:0:2,1,,,,,,1:0:4,1,,,,,,,,,,,,,1:0:4,/6,0=1:0:4,,3:0:4,,,,,1:0:4,,,,,,,,,,,,,,");
				paster.paste(*realm, chunk_position.topLeft() + Position{29, 23});
			}

			provider.updateChunk(chunk_position);
		});

		if (initial_generation)
			safeDynamicCast<ShipRealm>(realm)->worldgenParams = params;

		{
			Timer pathmap_timer("RemakePathMap");
			realm->remakePathMap(range);
		}

		ship_realm_timer.stop();
		if (initial_generation)
			Timer::summary();
		Timer::clear();
	}
}
