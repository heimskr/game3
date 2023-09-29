#include "graphics/Tileset.h"
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

	uint64_t TileProvider::updateChunk(ChunkPosition chunk_position) {
		std::unique_lock meta_lock(metaMutex);
		return ++metaMap[chunk_position].updateCount;
	}

	uint64_t TileProvider::getUpdateCounter(ChunkPosition chunk_position) {
		std::shared_lock shared_lock(metaMutex);
		if (auto iter = metaMap.find(chunk_position); iter != metaMap.end())
			return iter->second.updateCount;
		return 0;
	}

	void TileProvider::setUpdateCounter(ChunkPosition chunk_position, uint64_t counter) {
		std::unique_lock meta_lock(metaMutex);
		metaMap[chunk_position].updateCount = counter;
	}

	void TileProvider::absorb(ChunkPosition chunk_position, ChunkSet chunk_set) {
		if (chunk_set.terrain.size() != LAYER_COUNT)
			throw std::invalid_argument("ChunkSet has invalid number of terrain layers in TileProvider::absorb: " + std::to_string(chunk_set.terrain.size()));

		if (chunk_set.biomes.size() != CHUNK_SIZE * CHUNK_SIZE)
			throw std::invalid_argument("Invalid number of biome tiles in TileProvider::absorb: " + std::to_string(chunk_set.biomes.size()));

		if (chunk_set.fluids.size() != CHUNK_SIZE * CHUNK_SIZE)
			throw std::invalid_argument("Invalid number of fluid tiles in TileProvider::absorb: " + std::to_string(chunk_set.fluids.size()));

		for (size_t i = 0; i < LAYER_COUNT; ++i) {
			std::unique_lock lock(chunkMutexes[i]);
			chunkMaps[i][chunk_position] = std::move(chunk_set.terrain[i]);
		}

		{
			std::unique_lock lock(biomeMutex);
			biomeMap[chunk_position] = std::move(chunk_set.biomes);
		}

		{
			std::unique_lock lock(fluidMutex);
			fluidMap[chunk_position] = std::move(chunk_set.fluids);
		}

		{
			std::unique_lock lock(pathMutex);
			pathMap[chunk_position] = std::move(chunk_set.pathmap);
		}

		updateChunk(chunk_position);
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
				if (tileset->isLand(copyTileUnsafe(Layer::Terrain, Position(row, column), was_empty, TileMode::Throw)))
					if (auto fluid_tile = copyFluidTile(Position(row, column)); !fluid_tile || fluid_tile->level == 0)
						land_tiles[i++] = {row, column};
		return land_tiles;
	}

	TileID TileProvider::copyTile(Layer layer, Position position, bool &was_empty, TileMode mode) const {
		std::shared_lock lock(chunkMutexes[getIndex(layer)]);
		return copyTileUnsafe(layer, position, was_empty, mode);
	}

	TileID TileProvider::copyTileUnsafe(Layer layer, Position position, bool &was_empty, TileMode mode) const {
		was_empty = false;
		validateLayer(layer);

		const ChunkPosition chunk_position = getChunkPosition(position);

		const ChunkMap &map = chunkMaps[getIndex(layer)];

		if (auto iter = map.find(chunk_position); iter != map.end())
			return access(iter->second, remainder(position.row), remainder(position.column));

		if (mode == TileMode::ReturnEmpty) {
			was_empty = true;
			return 0;
		}

		throw std::out_of_range("Couldn't copy tile at " + std::string(position));
	}

	TileID TileProvider::copyTile(Layer layer, Position position, TileMode mode) const {
		bool was_empty = false;
		return copyTile(layer, position, was_empty, mode);
	}

	std::optional<TileID> TileProvider::tryTile(Layer layer, const Position &position) const {
		validateLayer(layer);

		const ChunkPosition chunk_position{divide(position.column), divide(position.row)};

		std::shared_lock lock(chunkMutexes[getIndex(layer)]);
		const auto &map = chunkMaps[getIndex(layer)];

		if (auto iter = map.find(chunk_position); iter != map.end())
			return access(iter->second, remainder(position.row), remainder(position.column));

		return std::nullopt;
	}

	std::optional<BiomeType> TileProvider::copyBiomeType(Position position) const {
		const ChunkPosition chunk_position{divide(position.column), divide(position.row)};

		std::shared_lock lock(biomeMutex);

		if (auto iter = biomeMap.find(chunk_position); iter != biomeMap.end())
			return access(iter->second, remainder(position.row), remainder(position.column));

		return std::nullopt;
	}

	std::optional<uint8_t> TileProvider::copyPathState(Position position) const {
		const ChunkPosition chunk_position{divide(position.column), divide(position.row)};

		std::shared_lock lock(pathMutex);

		if (auto iter = pathMap.find(chunk_position); iter != pathMap.end())
			return access(iter->second, remainder(position.row), remainder(position.column));

		return std::nullopt;
	}

	std::optional<FluidTile> TileProvider::copyFluidTile(Position position) const {
		const ChunkPosition chunk_position{divide(position.column), divide(position.row)};

		std::shared_lock lock(fluidMutex);

		if (auto iter = fluidMap.find(chunk_position); iter != fluidMap.end())
			return access(iter->second, remainder(position.row), remainder(position.column));

		return std::nullopt;
	}

	ChunkSet TileProvider::getChunkSet(ChunkPosition chunk_position) const {
		std::vector<TileChunk> terrain;

		for (size_t i = 0; i < LAYER_COUNT; ++i) {
			std::shared_lock lock(chunkMutexes[i]);
			terrain.push_back(chunkMaps[i].at(chunk_position));
		}

		BiomeChunk biomes;

		{
			std::shared_lock lock(biomeMutex);
			biomes = biomeMap.at(chunk_position);
		}

		FluidChunk fluids;

		{
			std::shared_lock lock(fluidMutex);
			fluids = fluidMap.at(chunk_position);
		}

		PathChunk pathmap;

		{
			std::shared_lock lock(pathMutex);
			pathmap = pathMap.at(chunk_position);
		}

		return {std::move(terrain), std::move(biomes), std::move(fluids), std::move(pathmap)};
	}

	std::string TileProvider::getRawChunks(ChunkPosition chunk_position) const {
		std::string raw;
		raw.reserve(LAYER_COUNT * CHUNK_SIZE * CHUNK_SIZE * sizeof(TileID));

		for (Layer layer: allLayers) {
			std::shared_lock lock(chunkMutexes[getIndex(layer)]);
			const TileChunk &chunk = chunkMaps[getIndex(layer)].at(chunk_position);
			appendSpan(raw, std::span(chunk));
		}

		{
			std::shared_lock lock(biomeMutex);
			appendSpan(raw, std::span(biomeMap.at(chunk_position)));
		}

		{
			std::shared_lock lock(fluidMutex);
			std::vector<FluidInt> raw_fluids;
			raw_fluids.reserve(CHUNK_SIZE * CHUNK_SIZE);

			for (const FluidTile &tile: fluidMap.at(chunk_position))
				raw_fluids.emplace_back(tile);

			appendSpan(raw, std::span(raw_fluids));
		}

		return raw;
	}

	std::string TileProvider::getRawTerrain(ChunkPosition chunk_position) const {
		std::string raw;
		raw.reserve(LAYER_COUNT * CHUNK_SIZE * CHUNK_SIZE * sizeof(TileID));

		for (Layer layer: allLayers) {
			std::shared_lock lock(chunkMutexes[getIndex(layer)]);
			const TileChunk &chunk = chunkMaps[getIndex(layer)].at(chunk_position);
			appendSpan(raw, std::span(chunk));
		}

		return raw;
	}

	std::string TileProvider::getRawBiomes(ChunkPosition chunk_position) const {
		std::string raw;
		raw.reserve(sizeof(BiomeType) * CHUNK_SIZE * CHUNK_SIZE);
		{
			std::shared_lock lock(biomeMutex);
			const BiomeChunk &biome_data = biomeMap.at(chunk_position);
			auto biome_lock = biome_data.sharedLock();
			appendSpan(raw, std::span(biome_data));
		}
		return raw;
	}

	std::string TileProvider::getRawPathmap(ChunkPosition chunk_position) const {
		std::string raw;
		raw.reserve(sizeof(uint8_t) * CHUNK_SIZE * CHUNK_SIZE);
		{
			std::shared_lock lock(pathMutex);
			const PathChunk &pathmap_data = pathMap.at(chunk_position);
			auto pathmap_lock = pathmap_data.sharedLock();
			appendSpan(raw, std::span(pathmap_data));
		}
		return raw;
	}

	std::string TileProvider::getRawFluids(ChunkPosition chunk_position) const {
		std::vector<FluidInt> raw_fluids;
		std::string raw;
		raw_fluids.reserve(CHUNK_SIZE * CHUNK_SIZE);
		raw.reserve(CHUNK_SIZE * CHUNK_SIZE * sizeof(FluidInt));
		{
			std::shared_lock lock(fluidMutex);
			const FluidChunk &fluid_data = fluidMap.at(chunk_position);
			auto fluid_lock = fluid_data.sharedLock();
			for (const FluidTile &tile: fluidMap.at(chunk_position))
				raw_fluids.emplace_back(tile);
		}
		assert(raw_fluids.size() == CHUNK_SIZE * CHUNK_SIZE);
		appendSpan(raw, std::span(raw_fluids));
		assert(raw.size() == CHUNK_SIZE * CHUNK_SIZE * sizeof(FluidInt));
		return raw;
	}

	TileID & TileProvider::findTile(Layer layer, Position position, bool &created, std::shared_lock<std::shared_mutex> *lock_out, TileMode mode) {
		created = false;
		validateLayer(layer);

		const ChunkPosition chunk_position {divide(position.column), divide(position.row)};

		std::shared_mutex &mutex = chunkMutexes[getIndex(layer)];
		std::shared_lock shared_lock(mutex);
		ChunkMap &map = chunkMaps[getIndex(layer)];

		if (auto iter = map.find(chunk_position); iter != map.end()) {
			if (lock_out != nullptr)
				*lock_out = std::move(shared_lock);
			return access(iter->second, remainder(position.row), remainder(position.column));
		}

		if (mode == TileMode::Create) {
			created = true;
			shared_lock.unlock();
			std::unique_lock unique_lock(mutex);
			TileChunk &chunk = map[chunk_position];
			initTileChunk(layer, chunk, chunk_position);
			TileID &accessed = access(chunk, remainder(position.row), remainder(position.column)) = 0;

			if (lock_out != nullptr) {
				unique_lock.unlock();
				shared_lock.lock();
				*lock_out = std::move(shared_lock);
			}

			return accessed;
		}

		throw std::out_of_range("Couldn't find tile at " + std::string(position));
	}

	TileID & TileProvider::findTile(Layer layer, Position position, bool &created, std::unique_lock<std::shared_mutex> *lock_out, TileMode mode) {
		created = false;
		validateLayer(layer);

		const ChunkPosition chunk_position {divide(position.column), divide(position.row)};

		std::shared_mutex &mutex = chunkMutexes[getIndex(layer)];
		std::shared_lock shared_lock(mutex);
		ChunkMap &map = chunkMaps[getIndex(layer)];

		if (auto iter = map.find(chunk_position); iter != map.end()) {
			if (lock_out != nullptr) {
				shared_lock.unlock();
				*lock_out = std::unique_lock(mutex);
			}
			return access(iter->second, remainder(position.row), remainder(position.column));
		}

		if (mode == TileMode::Create) {
			created = true;
			shared_lock.unlock();
			std::unique_lock unique_lock(mutex);
			TileChunk &chunk = map[chunk_position];
			initTileChunk(layer, chunk, chunk_position);
			TileID &accessed = access(chunk, remainder(position.row), remainder(position.column)) = 0;
			if (lock_out != nullptr)
				*lock_out = std::move(unique_lock);
			return accessed;
		}

		throw std::out_of_range("Couldn't find tile at " + std::string(position));
	}

	BiomeType & TileProvider::findBiomeType(Position position, bool &created, std::shared_lock<std::shared_mutex> *lock_out, BiomeMode mode) {
		created = false;

		const ChunkPosition chunk_position {divide(position.column), divide(position.row)};

		std::shared_lock shared_lock(biomeMutex);

		if (auto iter = biomeMap.find(chunk_position); iter != biomeMap.end()) {
			if (lock_out != nullptr)
				*lock_out = std::move(shared_lock);
			return access(iter->second, remainder(position.row), remainder(position.column));
		}

		if (mode == BiomeMode::Create) {
			created = true;
			shared_lock.unlock();
			std::unique_lock unique_lock(biomeMutex);
			BiomeChunk &chunk = biomeMap[chunk_position];
			initBiomeChunk(chunk, chunk_position);
			BiomeType &accessed = access(chunk, remainder(position.row), remainder(position.column)) = 0;

			// Transfer the lock sketchily.
			if (lock_out != nullptr) {
				unique_lock.unlock();
				// Yikes.
				shared_lock.lock();
				*lock_out = std::move(shared_lock);
			}

			return accessed;
		}

		throw std::out_of_range("Couldn't find biome type at " + std::string(position));
	}

	BiomeType & TileProvider::findBiomeType(Position position, bool &created, std::unique_lock<std::shared_mutex> *lock_out, BiomeMode mode) {
		created = false;

		const ChunkPosition chunk_position {divide(position.column), divide(position.row)};

		std::shared_lock shared_lock(biomeMutex);

		if (auto iter = biomeMap.find(chunk_position); iter != biomeMap.end()) {
			if (lock_out != nullptr) {
				shared_lock.unlock();
				*lock_out = std::unique_lock(pathMutex);
			}

			return access(iter->second, remainder(position.row), remainder(position.column));
		}

		if (mode == BiomeMode::Create) {
			created = true;
			shared_lock.unlock();
			std::unique_lock unique_lock(biomeMutex);
			BiomeChunk &chunk = biomeMap[chunk_position];
			initBiomeChunk(chunk, chunk_position);
			BiomeType &accessed = access(chunk, remainder(position.row), remainder(position.column)) = 0;
			if (lock_out != nullptr)
				*lock_out = std::move(unique_lock);
			return accessed;
		}

		throw std::out_of_range("Couldn't find biome type at " + std::string(position));
	}

	uint8_t & TileProvider::findPathState(Position position, bool &created, std::shared_lock<std::shared_mutex> *lock_out, PathMode mode) {
		created = false;

		const ChunkPosition chunk_position {divide(position.column), divide(position.row)};

		std::shared_lock shared_lock(pathMutex);

		if (auto iter = pathMap.find(chunk_position); iter != pathMap.end()) {
			if (lock_out != nullptr)
				*lock_out = std::move(shared_lock);
			return access(iter->second, remainder(position.row), remainder(position.column));
		}

		if (mode == PathMode::Create) {
			created = true;
			shared_lock.unlock();
			std::unique_lock unique_lock(pathMutex);
			PathChunk &chunk = pathMap[chunk_position];
			initPathChunk(chunk, chunk_position);
			uint8_t &accessed = access(chunk, remainder(position.row), remainder(position.column)) = 0;

			// Transfer the lock sketchily.
			if (lock_out != nullptr) {
				unique_lock.unlock();
				// Yikes.
				shared_lock.lock();
				*lock_out = std::move(shared_lock);
			}

			return accessed;
		}

		throw std::out_of_range("Couldn't find path state at " + std::string(position));
	}

	uint8_t & TileProvider::findPathState(Position position, bool &created, std::unique_lock<std::shared_mutex> *lock_out, PathMode mode) {
		created = false;

		const ChunkPosition chunk_position {divide(position.column), divide(position.row)};

		std::shared_lock shared_lock(pathMutex);

		if (auto iter = pathMap.find(chunk_position); iter != pathMap.end()) {
			if (lock_out != nullptr) {
				shared_lock.unlock();
				*lock_out = std::unique_lock(pathMutex);
			}

			return access(iter->second, remainder(position.row), remainder(position.column));
		}

		if (mode == PathMode::Create) {
			created = true;
			shared_lock.unlock();
			std::unique_lock unique_lock(pathMutex);
			PathChunk &chunk = pathMap[chunk_position];
			initPathChunk(chunk, chunk_position);
			uint8_t &accessed = access(chunk, remainder(position.row), remainder(position.column));
			if (lock_out != nullptr)
				*lock_out = std::move(unique_lock);
			return accessed = 0;
		}

		throw std::out_of_range("Couldn't find path state at " + std::string(position));
	}

	FluidTile & TileProvider::findFluid(Position position, std::shared_lock<std::shared_mutex> *lock_out, FluidMode mode) {
		const auto chunk_position = getChunkPosition(position);
		std::shared_lock shared_lock(fluidMutex);

		if (auto iter = fluidMap.find(chunk_position); iter != fluidMap.end()) {
			if (lock_out != nullptr)
				*lock_out = std::move(shared_lock);
			return access(iter->second, remainder(position.row), remainder(position.column));
		}

		if (mode == FluidMode::Create) {
			shared_lock.unlock();
			std::unique_lock unique_lock(fluidMutex);
			auto &chunk = fluidMap[chunk_position];
			initFluidChunk(chunk, chunk_position);
			FluidTile &accessed = access(chunk, remainder(position.row), remainder(position.column)) = {0, 0};

			if (lock_out != nullptr) {
				unique_lock.unlock();
				shared_lock.lock();
				*lock_out = std::move(shared_lock);
			}

			return accessed;
		}

		throw std::out_of_range("Couldn't find fluid tile at " + static_cast<std::string>(position));
	}

	FluidTile & TileProvider::findFluid(Position position, std::unique_lock<std::shared_mutex> *lock_out, FluidMode mode) {
		const auto chunk_position = getChunkPosition(position);
		std::shared_lock shared_lock(fluidMutex);

		if (auto iter = fluidMap.find(chunk_position); iter != fluidMap.end()) {
			if (lock_out != nullptr) {
				shared_lock.unlock();
				*lock_out = std::unique_lock(fluidMutex);
			}
			return access(iter->second, remainder(position.row), remainder(position.column));
		}

		if (mode == FluidMode::Create) {
			shared_lock.unlock();
			std::unique_lock unique_lock(fluidMutex);
			auto &chunk = fluidMap[chunk_position];
			initFluidChunk(chunk, chunk_position);
			FluidTile &accessed = access(chunk, remainder(position.row), remainder(position.column)) = {0, 0};
			if (lock_out != nullptr)
				*lock_out = std::move(unique_lock);
			return accessed;
		}

		throw std::out_of_range("Couldn't find fluid tile at " + static_cast<std::string>(position));
	}

	const TileChunk & TileProvider::getTileChunk(Layer layer, ChunkPosition chunk_position) const {
		validateLayer(layer);

		std::shared_lock lock(chunkMutexes[getIndex(layer)]);
		if (auto iter = chunkMaps[getIndex(layer)].find(chunk_position); iter != chunkMaps[getIndex(layer)].end())
			return iter->second;

		throw std::out_of_range("Couldn't find tile chunk at position " + static_cast<std::string>(chunk_position));
	}

	TileChunk & TileProvider::getTileChunk(Layer layer, ChunkPosition chunk_position) {
		validateLayer(layer);
		ensureTileChunk(chunk_position, layer);
		std::unique_lock lock(chunkMutexes[getIndex(layer)]);
		return chunkMaps[getIndex(layer)][chunk_position];
	}

	std::optional<std::reference_wrapper<const TileChunk>> TileProvider::tryTileChunk(Layer layer, ChunkPosition chunk_position) const {
		std::shared_lock lock(chunkMutexes[getIndex(layer)]);
		if (auto iter = chunkMaps[getIndex(layer)].find(chunk_position); iter != chunkMaps[getIndex(layer)].end())
			return std::ref(iter->second);
		return std::nullopt;
	}

	std::optional<std::reference_wrapper<TileChunk>> TileProvider::tryTileChunk(Layer layer, ChunkPosition chunk_position) {
		std::shared_lock lock(chunkMutexes[getIndex(layer)]);
		if (auto iter = chunkMaps[getIndex(layer)].find(chunk_position); iter != chunkMaps[getIndex(layer)].end())
			return std::ref(iter->second);
		return std::nullopt;
	}

	const Chunk<BiomeType> & TileProvider::getBiomeChunk(ChunkPosition chunk_position) const {
		std::shared_lock lock(biomeMutex);
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
		std::shared_lock lock(pathMutex);
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
		std::shared_lock lock(fluidMutex);
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

	void TileProvider::ensureAllChunks(Position position) {
		ensureAllChunks(getChunkPosition(position));
	}

	void TileProvider::validateLayer(Layer layer) const {
		if (static_cast<uint8_t>(layer) < static_cast<uint8_t>(Layer::Terrain) || LAYER_COUNT < static_cast<uint8_t>(layer))
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

	void TileProvider::toJSON(nlohmann::json &json, bool full_data) const {
		json["tilesetID"] = tilesetID;

		if (full_data) {
			auto &data = json["data"];

			nlohmann::json tile_array;
			for (const auto layer: allLayers) {
				std::shared_lock biome_lock(chunkMutexes[getIndex(layer)]);
				for (auto &[position, chunk]: chunkMaps[getIndex(layer)]) {
					auto chunk_lock = chunk.sharedLock();
					tile_array.push_back(std::make_pair(std::make_tuple(getIndex(layer), position.x, position.y), compress(std::span(chunk.data(), chunk.size()))));
				}
			}
			data.push_back(std::move(tile_array));

			nlohmann::json biome_array;
			{
				std::shared_lock biome_lock(biomeMutex);
				for (auto &[position, chunk]: biomeMap) {
					auto chunk_lock = chunk.sharedLock();
					biome_array.push_back(std::make_pair(std::make_pair(position.x, position.y), compress(std::span(chunk.data(), chunk.size()))));
				}
			}
			data.push_back(std::move(biome_array));

			nlohmann::json path_array;
			{
				std::shared_lock path_lock(pathMutex);
				for (auto &[position, chunk]: pathMap) {
					auto chunk_lock = chunk.sharedLock();
					path_array.push_back(std::make_pair(std::make_pair(position.x, position.y), compress(std::span(chunk.data(), chunk.size()))));
				}
			}
			data.push_back(std::move(path_array));

			nlohmann::json fluid_array;
			{
				static_assert(sizeof(FluidLevel) == 2);
				std::shared_lock fluid_lock(fluidMutex);
				for (auto &[position, chunk]: fluidMap) {
					std::vector<FluidInt> packed;
					{
						auto chunk_lock = chunk.sharedLock();
						packed.reserve(chunk.size());
						for (const FluidTile &fluid_tile: chunk)
							packed.emplace_back(fluid_tile);
					}
					fluid_array.push_back(std::make_pair(std::make_pair(position.x, position.y), compress(std::span(packed.data(), packed.size()))));
				}
			}
			data.push_back(std::move(fluid_array));
		}
	}

	void TileProvider::absorbJSON(const nlohmann::json &json, bool full_data) {
		tilesetID = json.at("tilesetID");

		if (full_data) {
			const nlohmann::json &data = json.at("data");

			for (const auto &item: data.at(0)) {
				const auto [layer, x, y] = item.at(0).get<std::tuple<size_t, int32_t, int32_t>>();
				static_assert(sizeof(TileID) == 2);
				const auto compressed = item.at(1).get<std::vector<uint8_t>>();
				chunkMaps[layer][ChunkPosition{x, y}] = decompress16(std::span(compressed.data(), compressed.size()));
			}

			for (const auto &item: data.at(1)) {
				const auto [x, y] = item.at(0).get<std::pair<int32_t, int32_t>>();
				static_assert(sizeof(BiomeType) == 2);
				const auto compressed = item.at(1).get<std::vector<uint8_t>>();
				biomeMap[ChunkPosition{x, y}] = decompress16(std::span(compressed.data(), compressed.size()));
			}

			for (const auto &item: data.at(2)) {
				const auto [x, y] = item.at(0).get<std::pair<int32_t, int32_t>>();
				static_assert(sizeof(PathChunk::value_type) == 1);
				const auto compressed = item.at(1).get<std::vector<uint8_t>>();
				pathMap[ChunkPosition{x, y}] = decompress8(std::span(compressed.data(), compressed.size()));
			}

			for (const auto &item: data.at(3)) {
				const auto [x, y] = item.at(0).get<std::pair<int32_t, int32_t>>();
				static_assert(sizeof(FluidTile) == 6);
				const auto compressed = item.at(1).get<std::vector<uint8_t>>();
				const auto decompressed = decompress64(std::span(compressed.data(), compressed.size()));
				auto &chunk = fluidMap[ChunkPosition{x, y}];
				chunk.clear();
				chunk.reserve(decompressed.size());
				for (const auto tile: decompressed) {
					chunk.emplace_back(
						(tile & 0xff) | ((tile >> 8) & 0xff), ((tile >> 16) & 0xff) | ((tile >> 24) & 0xff) | ((tile >> 32) & 0xff) | ((tile >> 40) & 0xff)
					);
				}
			}
		}
	}

	ChunkPosition getChunkPosition(Position position) {
		return {TileProvider::divide<int32_t>(position.column), TileProvider::divide<int32_t>(position.row)};
	}
}
