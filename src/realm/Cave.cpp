#include "Log.h"
#include "algorithm/Voronoi.h"
#include "graphics/Tileset.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "realm/Cave.h"
#include "tileentity/Building.h"
#include "util/Util.h"
#include "worldgen/CaveGen.h"

#include <ranges>

namespace Game3 {
	Cave::Cave(const GamePtr &game_, RealmID id_, RealmID parent_realm, int seed_):
		Realm(game_, id_, ID(), "base:tileset/monomap", seed_), parentRealm(parent_realm) {}

	void Cave::clearLighting(float) {
		glClearColor(0.117, 0.117, 0.235, 1); CHECKGL
		// glClearColor(1, 1, 1, 1); CHECKGL
		glClear(GL_COLOR_BUFFER_BIT); CHECKGL
	}

	void Cave::onRemove() {
		// Assumptions:
		// - All entrances to a given cave realm appear in exactly one realm.
		//    -> If we find a cave entrance in one realm, we don't need to search other realms for another entrance to the same cave.
		// - All cave entrances in a given realm lead to the same cave.
		//    -> If we find one cave entrance in a realm, we can stop after destroying its linked cave and we don't have to look for more entrances.
		GamePtr game = getGame();
		auto lock = tileEntities.sharedLock();
		for (const auto &[index, tile_entity]: tileEntities) {
			if (tile_entity->tileID != "base:tile/cave")
				continue;

			if (auto building = std::dynamic_pointer_cast<Building>(tile_entity)) {
				if (auto cave_realm = std::dynamic_pointer_cast<Cave>(game->getRealm(building->innerRealmID)))
					game->removeRealm(cave_realm);
				else
					WARN("Cave entrance leads to realm {}, which isn't a cave. Not erasing.", building->innerRealmID);
				break;
			}
		}
	}

	void Cave::reveal(const Position &position, bool force) {
		const auto &tileset = getTileset();
		const TileID empty_id = tileset.getEmptyID();
		if (force || getTile(Layer::Objects, position) != empty_id) {
			const TileID void_id = tileset["base:tile/void"];
			for (Index row_offset = -1; row_offset <= 1; ++row_offset)
				for (Index column_offset = -1; column_offset <= 1; ++column_offset)
					if (row_offset != 0 || column_offset != 0)
						if (const Position offset_position = position + Position(row_offset, column_offset); getTile(Layer::Highest, offset_position) == void_id)
							setTile(Layer::Highest, offset_position, empty_id);
		}
	}

	void Cave::generateChunk(const ChunkPosition &chunk_position) {
		auto rng = chunk_position.getRNG();
		tileProvider.ensureAllChunks(chunk_position);
		WorldGen::generateCave(shared_from_this(), rng, seed, {chunk_position, chunk_position});
		tileProvider.updateChunk(chunk_position);
	}

	RectangularVector<uint16_t> & Cave::getOreVoronoi(ChunkPosition chunk_position, std::unique_lock<DefaultMutex> &lock, std::default_random_engine &rng) {
		auto outer_lock = oreVoronoi.uniqueLock();

		if (auto iter = oreVoronoi.find(chunk_position); iter != oreVoronoi.end()) {
			lock = iter->second.uniqueLock();
			return iter->second;
		}

		Tileset &tileset = getTileset();

		auto [iter, inserted] = oreVoronoi.try_emplace(chunk_position, CHUNK_SIZE, CHUNK_SIZE);

		RectangularVector<uint16_t> &vector = iter->second.getBase();

		const auto &rare_category = tileset.getTilesByCategory("base:category/rare_ores");

		std::vector<TileID> rares;
		rares.reserve(rare_category.size());

		for (const Identifier &rare_ore: rare_category)
			rares.push_back(tileset[rare_ore]);

		voronoize<uint16_t>(vector, safeCast<int>(rares.size()), rng, [&] {
			return safeCast<uint16_t>(choose(rares, rng));
		});

		lock = iter->second.uniqueLock();
		return vector;
	}

	void Cave::absorbJSON(const nlohmann::json &json, bool full_data) {
		Realm::absorbJSON(json, full_data);
		parentRealm = json.at("parentRealm");
		if (auto iter = json.find("entranceCount"); iter != json.end())
			entranceCount = iter->get<size_t>();
		else
			entranceCount = 1;
	}

	void Cave::toJSON(nlohmann::json &json, bool full_data) const {
		Realm::toJSON(json, full_data);
		json["parentRealm"] = parentRealm;
		if (entranceCount != 1)
			json["entranceCount"] = entranceCount.load();
	}
}
