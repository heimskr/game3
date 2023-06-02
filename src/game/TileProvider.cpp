#include "Tileset.h"
#include "game/Game.h"
#include "game/TileProvider.h"
#include "util/Util.h"
#include "util/Zstd.h"

namespace Game3 {
	TileProvider::TileProvider(Identifier tileset_id):
		tilesetID(std::move(tileset_id)) {}

	void TileProvider::clear() {
		for (auto &map: chunkMaps)
			map.clear();
		biomeMap.clear();
	}

	bool TileProvider::contains(ChunkPosition chunk_position) const {
		if (!pathMap.contains(chunk_position) || !biomeMap.contains(chunk_position))
			return false;
		for (const auto &map: chunkMaps)
			if (!map.contains(chunk_position))
				return false;
		return true;
	}

	std::shared_ptr<Tileset> TileProvider::getTileset(const Game &game) {
		if (cachedTileset)
			return cachedTileset;

		if (!tilesetID)
			throw std::runtime_error("Can't get empty tileset from TileProvider");

		return cachedTileset = game.registry<TilesetRegistry>().at(tilesetID);
	}

	std::vector<Position> TileProvider::getLand(const Game &game, const ChunkRange &range, Index right_pad, Index bottom_pad) {
		const auto tileset = getTileset(game);
		std::vector<Position> land_tiles;
		land_tiles.resize((range.tileWidth() - right_pad) * (range.tileHeight() - bottom_pad));

		std::shared_lock lock(chunkMutexes[0]);

		size_t i = 0;
		bool was_empty = false;

		for (Index row = range.rowMin(); row <= range.rowMax() - bottom_pad; ++row)
			for (Index column = range.columnMin(); column < range.columnMax() - right_pad; ++column)
				if (tileset->isLand(copyTileUnsafe(Layer::Terrain, row, column, was_empty, TileMode::Throw)))
					land_tiles[i++] = {row, column};
		return land_tiles;
	}

	TileID TileProvider::copyTile(Layer layer, Index row, Index column, bool &was_empty, TileMode mode) const {
		std::shared_lock lock(const_cast<std::shared_mutex &>(chunkMutexes[getIndex(layer)]));
		return copyTileUnsafe(layer, row, column, was_empty, mode);
	}

	TileID TileProvider::copyTileUnsafe(Layer layer, Index row, Index column, bool &was_empty, TileMode mode) const {
		was_empty = false;
		validateLayer(layer);

		const ChunkPosition chunk_position {divide<int32_t>(column), divide<int32_t>(row)};

		const auto &map = chunkMaps[getIndex(layer)];

		if (auto iter = map.find(chunk_position); iter != map.end())
			return access(iter->second, remainder(row), remainder(column));

		if (mode == TileMode::ReturnEmpty) {
			was_empty = true;
			return 0;
		}

		throw std::out_of_range("Couldn't copy tile at (" + std::to_string(row) + ", " + std::to_string(column) + ')');
	}

	TileID TileProvider::copyTile(Layer layer, Index row, Index column, TileMode mode) const {
		bool was_empty = false;
		return copyTile(layer, row, column, was_empty, mode);
	}

	std::optional<TileID> TileProvider::tryTile(Layer layer, const Position &position) const {
		validateLayer(layer);

		const ChunkPosition chunk_position {divide(position.column), divide(position.row)};

		std::shared_lock lock(const_cast<std::shared_mutex &>(chunkMutexes[getIndex(layer)]));
		const auto &map = chunkMaps[getIndex(layer)];

		if (auto iter = map.find(chunk_position); iter != map.end())
			return access(iter->second, remainder(position.row), remainder(position.column));

		return std::nullopt;
	}

	std::optional<BiomeType> TileProvider::copyBiomeType(Index row, Index column) const {
		const ChunkPosition chunk_position {divide(column), divide(row)};

		std::shared_lock lock(const_cast<std::shared_mutex &>(biomeMutex));

		if (auto iter = biomeMap.find(chunk_position); iter != biomeMap.end())
			return access(iter->second, remainder(row), remainder(column));

		return std::nullopt;
	}

	std::optional<uint8_t> TileProvider::copyPathState(Index row, Index column) const {
		const ChunkPosition chunk_position {divide(column), divide(row)};

		std::shared_lock lock(const_cast<std::shared_mutex &>(pathMutex));

		if (auto iter = pathMap.find(chunk_position); iter != pathMap.end())
			return access(iter->second, remainder(row), remainder(column));

		return std::nullopt;
	}

	TileID & TileProvider::findTile(Layer layer, Index row, Index column, bool &created, TileMode mode) {
		created = false;
		validateLayer(layer);

		const ChunkPosition chunk_position {divide(column), divide(row)};

		std::shared_lock shared_lock(chunkMutexes[getIndex(layer)]);
		auto &map = chunkMaps[getIndex(layer)];

		if (auto iter = map.find(chunk_position); iter != map.end())
			return access(iter->second, remainder(row), remainder(column));

		if (mode == TileMode::Create) {
			created = true;
			shared_lock.unlock();
			std::unique_lock unique_lock(chunkMutexes[getIndex(layer)]);
			auto &chunk = map[chunk_position];
			initTileChunk(layer, chunk, chunk_position);
			return access(chunk, remainder(row), remainder(column)) = 0;
		}

		throw std::out_of_range("Couldn't find tile at (" + std::to_string(row) + ", " + std::to_string(column) + ')');
	}

	TileID & TileProvider::findTile(Layer layer, Index row, Index column, TileMode mode) {
		bool created = false;
		return findTile(layer, row, column, created, mode);
	}

	BiomeType & TileProvider::findBiomeType(Index row, Index column, bool &created, BiomeMode mode) {
		created = false;

		const ChunkPosition chunk_position {divide(column), divide(row)};

		std::shared_lock shared_lock(biomeMutex);

		if (auto iter = biomeMap.find(chunk_position); iter != biomeMap.end())
			return access(iter->second, remainder(row), remainder(column));

		if (mode == BiomeMode::Create) {
			created = true;
			shared_lock.unlock();
			std::unique_lock unique_lock(biomeMutex);
			auto &chunk = biomeMap[chunk_position];
			initBiomeChunk(chunk, chunk_position);
			return access(chunk, remainder(row), remainder(column)) = 0;
		}

		throw std::out_of_range("Couldn't find biome type at (" + std::to_string(row) + ", " + std::to_string(column) + ')');
	}

	BiomeType & TileProvider::findBiomeType(Index row, Index column, BiomeMode mode) {
		bool created = false;
		return findBiomeType(row, column, created, mode);
	}

	uint8_t & TileProvider::findPathState(Index row, Index column, bool &created, PathMode mode) {
		created = false;

		const ChunkPosition chunk_position {divide(column), divide(row)};

		std::shared_lock shared_lock(pathMutex);

		if (auto iter = pathMap.find(chunk_position); iter != pathMap.end())
			return access(iter->second, remainder(row), remainder(column));

		if (mode == PathMode::Create) {
			created = true;
			shared_lock.unlock();
			std::unique_lock unique_lock(pathMutex);
			auto &chunk = pathMap[chunk_position];
			initPathChunk(chunk, chunk_position);
			return access(chunk, remainder(row), remainder(column)) = 0;
		}

		throw std::out_of_range("Couldn't find path state at (" + std::to_string(row) + ", " + std::to_string(column) + ')');
	}

	uint8_t & TileProvider::findPathState(Index row, Index column, PathMode mode) {
		bool created = false;
		return findPathState(row, column, created, mode);
	}

	const Chunk<TileID> & TileProvider::getTileChunk(Layer layer, ChunkPosition chunk_position) const {
		validateLayer(layer);

		std::shared_lock lock(const_cast<std::shared_mutex &>(chunkMutexes[getIndex(layer)]));
		if (auto iter = chunkMaps[getIndex(layer)].find(chunk_position); iter != chunkMaps[getIndex(layer)].end())
			return iter->second;

		throw std::out_of_range("Couldn't find tile chunk at position " + static_cast<std::string>(chunk_position));
	}

	Chunk<TileID> & TileProvider::getTileChunk(Layer layer, ChunkPosition chunk_position) {
		validateLayer(layer);
		ensureTileChunk(chunk_position, layer);
		std::unique_lock lock(chunkMutexes[getIndex(layer)]);
		return chunkMaps[getIndex(layer)][chunk_position];
	}

	const Chunk<BiomeType> & TileProvider::getBiomeChunk(ChunkPosition chunk_position) const {
		std::shared_lock lock(const_cast<std::shared_mutex &>(biomeMutex));
		if (auto iter = biomeMap.find(chunk_position); iter != biomeMap.end())
			return iter->second;

		throw std::out_of_range("Couldn't find biome chunk at position " + static_cast<std::string>(chunk_position));
	}

	Chunk<BiomeType> & TileProvider::getBiomeChunk(ChunkPosition chunk_position) {
		ensureBiomeChunk(chunk_position);
		std::unique_lock lock(biomeMutex);
		return biomeMap[chunk_position];
	}

	const Chunk<uint8_t> & TileProvider::getPathChunk(ChunkPosition chunk_position) const {
		std::shared_lock lock(const_cast<std::shared_mutex &>(pathMutex));
		if (auto iter = pathMap.find(chunk_position); iter != pathMap.end())
			return iter->second;

		throw std::out_of_range("Couldn't find path chunk at position " + static_cast<std::string>(chunk_position));
	}

	Chunk<uint8_t> & TileProvider::getPathChunk(ChunkPosition chunk_position) {
		ensurePathChunk(chunk_position);
		std::unique_lock lock(pathMutex);
		return pathMap[chunk_position];
	}

	const Chunk<FluidTile> & TileProvider::getFluidChunk(ChunkPosition chunk_position) const {
		std::shared_lock lock(const_cast<std::shared_mutex &>(fluidMutex));
		if (auto iter = fluidMap.find(chunk_position); iter != fluidMap.end())
			return iter->second;

		throw std::out_of_range("Couldn't find fluid chunk at position " + static_cast<std::string>(chunk_position));
	}

	Chunk<FluidTile> & TileProvider::getFluidChunk(ChunkPosition chunk_position) {
		ensureFluidChunk(chunk_position);
		std::unique_lock lock(fluidMutex);
		return fluidMap[chunk_position];
	}

	void TileProvider::ensureTileChunk(ChunkPosition chunk_position) {
		// for (Layer layer = 0; layer < LAYER_COUNT; ++layer) {
		for (const auto layer: allLayers) {
			std::unique_lock lock(chunkMutexes[getIndex(layer)]);
			if (auto [iter, inserted] = chunkMaps[getIndex(layer)].try_emplace(chunk_position); inserted)
				initTileChunk(layer, iter->second, chunk_position);
		}
	}

	void TileProvider::ensureTileChunk(ChunkPosition chunk_position, Layer layer) {
		validateLayer(layer);
		std::unique_lock lock(chunkMutexes[getIndex(layer)]);
		if (auto [iter, inserted] = chunkMaps[getIndex(layer)].try_emplace(chunk_position); inserted)
			initTileChunk(layer, iter->second, chunk_position);
	}

	void TileProvider::ensureBiomeChunk(ChunkPosition chunk_position) {
		std::unique_lock lock(biomeMutex);
		if (auto [iter, inserted] = biomeMap.try_emplace(chunk_position); inserted)
			initBiomeChunk(iter->second, chunk_position);
	}

	void TileProvider::ensurePathChunk(ChunkPosition chunk_position) {
		std::unique_lock lock(pathMutex);
		if (auto [iter, inserted] = pathMap.try_emplace(chunk_position); inserted)
			initPathChunk(iter->second, chunk_position);
	}

	void TileProvider::ensureFluidChunk(ChunkPosition chunk_position) {
		std::unique_lock lock(fluidMutex);
		if (auto [iter, inserted] = fluidMap.try_emplace(chunk_position); inserted)
			initFluidChunk(iter->second, chunk_position);
	}

	void TileProvider::ensureAllChunks(ChunkPosition chunk_position) {
		ensurePathChunk(chunk_position);
		ensureBiomeChunk(chunk_position);
		ensureTileChunk(chunk_position);
		ensureFluidChunk(chunk_position);
	}

	void TileProvider::ensureAllChunks(const Position &position) {
		ensureAllChunks(getChunkPosition(position.row, position.column));
	}

	void TileProvider::validateLayer(Layer layer) const {
		if (static_cast<uint8_t>(layer) < static_cast<uint8_t>(Layer::Terrain) || static_cast<uint8_t>(Layer::Highest) < static_cast<uint8_t>(layer))
			throw std::out_of_range("Invalid layer: " + std::to_string(static_cast<uint8_t>(layer)));
	}

	void TileProvider::initTileChunk(Layer, Chunk<TileID> &chunk, ChunkPosition chunk_position) {
		chunk.resize(CHUNK_SIZE * CHUNK_SIZE, 0);
		generationQueue.push(chunk_position);
	}

	void TileProvider::initBiomeChunk(Chunk<BiomeType> &chunk, ChunkPosition) {
		chunk.resize(CHUNK_SIZE * CHUNK_SIZE, 0);
	}

	void TileProvider::initPathChunk(Chunk<uint8_t> &chunk, ChunkPosition) {
		chunk.resize(CHUNK_SIZE * CHUNK_SIZE, 0);
	}

	void TileProvider::initFluidChunk(Chunk<FluidTile> &chunk, ChunkPosition) {
		chunk.resize(CHUNK_SIZE * CHUNK_SIZE, {0, 0});
	}

	void to_json(nlohmann::json &json, const TileProvider &provider) {
		auto &unconst = const_cast<TileProvider &>(provider);
		json.push_back(provider.tilesetID);

		nlohmann::json tile_array;
		for (const auto layer: allLayers) {
			std::shared_lock biome_lock(unconst.chunkMutexes[getIndex(layer)]);
			for (auto &[position, chunk]: unconst.chunkMaps[getIndex(layer)]) {
				auto chunk_lock = chunk.sharedLock();
				tile_array.push_back(std::make_pair(std::make_tuple(getIndex(layer), position.x, position.y), compress(std::span(chunk.data(), chunk.size()))));
			}
		}
		json.push_back(std::move(tile_array));

		nlohmann::json biome_array;
		{
			std::shared_lock biome_lock(unconst.biomeMutex);
			for (auto &[position, chunk]: unconst.biomeMap) {
				auto chunk_lock = chunk.sharedLock();
				biome_array.push_back(std::make_pair(std::make_pair(position.x, position.y), compress(std::span(chunk.data(), chunk.size()))));
			}
		}
		json.push_back(std::move(biome_array));

		nlohmann::json path_array;
		{
			std::shared_lock path_lock(unconst.pathMutex);
			for (auto &[position, chunk]: unconst.pathMap) {
				auto chunk_lock = chunk.sharedLock();
				path_array.push_back(std::make_pair(std::make_pair(position.x, position.y), compress(std::span(chunk.data(), chunk.size()))));
			}
		}
		json.push_back(std::move(path_array));

		nlohmann::json fluid_array;
		{
			static_assert(sizeof(FluidLevel) == 2);
			std::shared_lock fluid_lock(unconst.fluidMutex);
			for (auto &[position, chunk]: unconst.fluidMap) {
				std::vector<uint32_t> packed;
				{
					auto chunk_lock = chunk.sharedLock();
					packed.reserve(chunk.size());
					for (const auto &[fluid_id, level]: chunk)
						packed.push_back(static_cast<uint32_t>(fluid_id) | (static_cast<uint32_t>(level) << 16));
				}
				fluid_array.push_back(std::make_pair(std::make_pair(position.x, position.y), compress(std::span(packed.data(), packed.size()))));
			}
		}
		json.push_back(std::move(fluid_array));
	}

	void from_json(const nlohmann::json &json, TileProvider &provider) {
		provider.tilesetID = json.at(0);

		for (const auto &item: json.at(1)) {
			const auto [layer, x, y] = item.at(0).get<std::tuple<size_t, int32_t, int32_t>>();
			const auto compressed = item.at(1).get<std::vector<uint8_t>>();
			provider.chunkMaps[layer][ChunkPosition{x, y}] = decompress16(std::span(compressed.data(), compressed.size()));
		}

		for (const auto &item: json.at(2)) {
			const auto [x, y] = item.at(0).get<std::pair<int32_t, int32_t>>();
			const auto compressed = item.at(1).get<std::vector<uint8_t>>();
			provider.biomeMap[ChunkPosition{x, y}] = decompress32(std::span(compressed.data(), compressed.size()));
		}

		for (const auto &item: json.at(3)) {
			const auto [x, y] = item.at(0).get<std::pair<int32_t, int32_t>>();
			const auto compressed = item.at(1).get<std::vector<uint8_t>>();
			provider.pathMap[ChunkPosition{x, y}] = decompress8(std::span(compressed.data(), compressed.size()));
		}

		for (const auto &item: json.at(4)) {
			const auto [x, y] = item.at(0).get<std::pair<int32_t, int32_t>>();
			static_assert(sizeof(FluidTile) == sizeof(uint32_t));
			const auto compressed = item.at(1).get<std::vector<uint8_t>>();
			const auto decompressed = decompress32(std::span(compressed.data(), compressed.size()));
			auto &chunk = provider.fluidMap[ChunkPosition{x, y}];
			chunk = {};
			chunk.reserve(decompressed.size());
			for (const auto tile: decompressed)
				chunk.emplace_back((tile & 0xff) | ((tile >> 8) & 0xff), ((tile >> 16) & 0xff) | ((tile >> 24) & 0xff));
		}
	}

	ChunkPosition getChunkPosition(Index row, Index column) {
		return {TileProvider::divide(column), TileProvider::divide(row)};
	}

	ChunkPosition getChunkPosition(const Position &position) {
		return getChunkPosition(position.row, position.column);
	}
}
