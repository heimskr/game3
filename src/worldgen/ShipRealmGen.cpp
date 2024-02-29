#include "biome/Biome.h"
#include "entity/Ship.h"
#include "game/Game.h"
#include "graphics/Tileset.h"
#include "realm/ShipRealm.h"
#include "realm/Realm.h"
#include "tileentity/EntityBuilding.h"
#include "tools/Paster.h"
#include "util/Cast.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/ShipRealmGen.h"
#include "worldgen/WorldGen.h"

#include <thread>

namespace Game3::WorldGen {
	void generateShipRealmChunks(const RealmPtr &realm, size_t, const WorldGenParams &params, const ChunkRange &range, bool initial_generation, const std::shared_ptr<Ship> &parent) {
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
				paster.ingest("empty;planks;tall_wooden_wall;sand;wooden_wall;bed2;torch;barrel;rope;grate_nw;grate_ne;ship_wheel;grate_mw;grate_me;grate_sw;grate_se;rowboat_w;rowboat_e/0,0=1:0:2,3:0:2,,,,,1:0:2,1:0:4,,,,,,,,,,,,,/1,0=1:0:2,1:0:5,,,,,1:0:2,1:0:6,1,,,,,,,,1:0:7,1:8,1:0:7,1:0:6,1:0:4,/2,0=1:0:2,1,,,,,1:0:2,1,,,,,,,,,1:9,1:10,1,,,1:0:4,/3,0=1:0:2,1,,,,,,,,1:0:11,1,,,,,,1:12,1:13,1,,,,1:0:4,/4,0=1:0:2,1,,,,,1:0:4,1,,,,,,,,,1:14,1:15,1,,,1:0:4,/5,0=1:0:2,1,,,,,1:0:4,1:0:6,1:8,1,1:0:7,1,,1:0:16,1:0:17,1,,1:0:7,1,1:0:6,1:0:4,/6,0=1:0:4,3:0:4,,,,,1:0:4,,,,,,,,,,,,,,");
				paster.paste(*realm, chunk_position.topLeft() + Position{29, 23});

				TileEntity::spawn<EntityBuilding>(realm, "base:tile/rowboat_w", Position{34, 36}, parent->getGID());
				TileEntity::spawn<EntityBuilding>(realm, "base:tile/rowboat_e", Position{34, 37}, parent->getGID());
			} else {
				provider.updateChunk(chunk_position);
				return;
			}

			chunk_position.iterate([&](const Position &position) {
				for (Layer layer: allLayers)
					realm->autotile(position, layer);
			});

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
