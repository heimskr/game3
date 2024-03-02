#include "biome/Biome.h"
#include "entity/Ship.h"
#include "game/Game.h"
#include "graphics/Tileset.h"
#include "realm/ShipRealm.h"
#include "realm/Realm.h"
#include "tileentity/EntityBuilding.h"
#include "tools/Paster.h"
#include "util/Cast.h"
#include "util/FS.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/ShipRealmGen.h"
#include "worldgen/WorldGen.h"

#include <thread>

namespace Game3::WorldGen {
	namespace {
		const std::string & shipDeckTemplate() { static auto out = readFile("resources/templates/ship_deck.g3"); return out; }
		const std::string & shipHoldTemplate() { static auto out = readFile("resources/templates/ship_hold.g3"); return out; }
	}

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
				paster.ingest(shipDeckTemplate());
				paster.patch({
					{{"op", "test"}, {"path", "/innerRealmID"}, {"value", 0}},
					{{"op", "replace"}, {"path", "/innerRealmID"}, {"value", realm->getID()}},
				});
				paster.patch({
					{{"op", "test"}, {"path", "/targetEntity"}, {"value", 0}},
					{{"op", "replace"}, {"path", "/targetEntity"}, {"value", parent->getGID()}},
				});
				paster.paste(realm, chunk_position.topLeft() + Position{29, 23});
			} else if (chunk_position == ChunkPosition{2, 0}) {
				paster.ingest(shipHoldTemplate());
				paster.patch({
					{{"op", "test"}, {"path", "/innerRealmID"}, {"value", 0}},
					{{"op", "replace"}, {"path", "/innerRealmID"}, {"value", realm->getID()}},
				});
				paster.paste(realm, chunk_position.topLeft() + Position{29, 23});
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
