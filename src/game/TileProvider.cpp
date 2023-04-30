#include "game/Game.h"
#include "game/TileProvider.h"
#include "util/Util.h"
#include "util/Zstd.h"

namespace Game3 {
	void TileProvider::clear() {
		tilesetID = {};
		for (auto &map: chunkMaps)
			map.clear();
		biomeMap.clear();
	}

	std::shared_ptr<Tileset> TileProvider::getTileset(const Game &game) const {
		if (!tilesetID)
			throw std::runtime_error("Can't get empty tileset from TileProvider");
		return game.registry<TilesetRegistry>().at(tilesetID);
	}

	TileID TileProvider::copyTile(Layer layer, Index row, Index column, bool &was_empty, TileMode mode) const {
		was_empty = false;
		validateLayer(layer);

		const auto &map = chunkMaps[layer - 1];
		const ChunkPosition chunk_position {divide(column), divide(row)};

		if (auto iter = map.find(chunk_position); iter != map.end())
			return access(iter->second, remainder(row), remainder(column));

		if (mode == TileMode::ReturnEmpty) {
			was_empty = true;
			return 0;
		}

		throw std::out_of_range("Couldn't copy tile at (" + std::to_string(column) + ", " + std::to_string(row) + ')');
	}

	TileID TileProvider::copyTile(Layer layer, Index row, Index column, TileMode mode) const {
		bool was_empty = false;
		return copyTile(layer, row, column, was_empty, mode);
	}

	std::optional<BiomeType> TileProvider::copyBiomeType(Index row, Index column) const {
		const ChunkPosition chunk_position {divide(column), divide(row)};

		if (auto iter = biomeMap.find(chunk_position); iter != biomeMap.end())
			return access(iter->second, remainder(row), remainder(column));

		throw std::out_of_range("Couldn't copy biome type at (" + std::to_string(column) + ", " + std::to_string(row) + ')');
	}

	std::optional<uint8_t> TileProvider::copyPathState(Index row, Index column) const {
		const ChunkPosition chunk_position {divide(column), divide(row)};

		if (auto iter = pathMap.find(chunk_position); iter != pathMap.end())
			return access(iter->second, remainder(row), remainder(column));

		throw std::out_of_range("Couldn't copy path state at (" + std::to_string(column) + ", " + std::to_string(row) + ')');
	}

	TileID & TileProvider::findTile(Layer layer, Index row, Index column, bool &created, TileMode mode) {
		created = false;
		validateLayer(layer);

		auto &map = chunkMaps[layer - 1];
		const ChunkPosition chunk_position {divide(column), divide(row)};

		if (auto iter = map.find(chunk_position); iter != map.end())
			return access(iter->second, remainder(row), remainder(column));

		if (mode == TileMode::Create) {
			created = true;
			return access(map[chunk_position], remainder(row), remainder(column)) = 0;
		}

		throw std::out_of_range("Couldn't find tile at (" + std::to_string(column) + ", " + std::to_string(row) + ')');
	}

	TileID & TileProvider::findTile(Layer layer, Index row, Index column, TileMode mode) {
		bool created = false;
		return findTile(layer, row, column, created, mode);
	}

	BiomeType & TileProvider::findBiomeType(Index row, Index column, bool &created, BiomeMode mode) {
		created = false;

		const ChunkPosition chunk_position {divide(column), divide(row)};

		if (auto iter = biomeMap.find(chunk_position); iter != biomeMap.end())
			return access(iter->second, remainder(row), remainder(column));

		if (mode == BiomeMode::Create) {
			created = true;
			return access(biomeMap[chunk_position], remainder(row), remainder(column)) = 0;
		}

		throw std::out_of_range("Couldn't find biome type at (" + std::to_string(column) + ", " + std::to_string(row) + ')');
	}

	BiomeType & TileProvider::findBiomeType(Index row, Index column, BiomeMode mode) {
		bool created = false;
		return findBiomeType(row, column, created, mode);
	}

	uint8_t & TileProvider::findPathState(Index row, Index column, bool &created, PathMode mode) {
		created = false;

		const ChunkPosition chunk_position {divide(column), divide(row)};

		if (auto iter = pathMap.find(chunk_position); iter != pathMap.end())
			return access(iter->second, remainder(row), remainder(column));

		if (mode == PathMode::Create) {
			created = true;
			return access(pathMap[chunk_position], remainder(row), remainder(column)) = 0;
		}

		throw std::out_of_range("Couldn't find biome type at (" + std::to_string(column) + ", " + std::to_string(row) + ')');
	}

	uint8_t & TileProvider::findPathState(Index row, Index column, PathMode mode) {
		bool created = false;
		return findPathState(row, column, created, mode);
	}

	const Chunk<TileID> & TileProvider::getTileChunk(Layer layer, const ChunkPosition &chunk_position) const {
		validateLayer(layer);

		if (auto iter = chunkMaps[layer - 1].find(chunk_position); iter != chunkMaps[layer - 1].end())
			return iter->second;

		throw std::out_of_range("Couldn't find tile chunk at position " + static_cast<std::string>(chunk_position));
	}

	Chunk<TileID> & TileProvider::getTileChunk(Layer layer, const ChunkPosition &chunk_position) {
		validateLayer(layer);
		ensureTileChunk(chunk_position, layer);
		return chunkMaps[layer - 1][chunk_position];
	}

	const Chunk<BiomeType> & TileProvider::getBiomeChunk(const ChunkPosition &chunk_position) const {
		if (auto iter = biomeMap.find(chunk_position); iter != biomeMap.end())
			return iter->second;

		throw std::out_of_range("Couldn't find biome chunk at position " + static_cast<std::string>(chunk_position));
	}

	Chunk<BiomeType> & TileProvider::getBiomeChunk(const ChunkPosition &chunk_position) {
		ensureBiomeChunk(chunk_position);
		return biomeMap[chunk_position];
	}

	const Chunk<uint8_t> & TileProvider::getPathChunk(const ChunkPosition &chunk_position) const {
		if (auto iter = pathMap.find(chunk_position); iter != pathMap.end())
			return iter->second;

		throw std::out_of_range("Couldn't find path chunk at position " + static_cast<std::string>(chunk_position));
	}

	Chunk<uint8_t> & TileProvider::getPathChunk(const ChunkPosition &chunk_position) {
		ensurePathChunk(chunk_position);
		return pathMap[chunk_position];
	}

	void TileProvider::ensureTileChunk(const ChunkPosition &chunk_position) {
		for (Layer layer = 0; layer < LAYER_COUNT; ++layer)
			chunkMaps[layer].try_emplace(chunk_position);
	}

	void TileProvider::ensureTileChunk(const ChunkPosition &chunk_position, Layer except) {
		for (Layer layer = 0; layer < LAYER_COUNT; ++layer)
			if (layer != except - 1)
				chunkMaps[layer].try_emplace(chunk_position);
	}

	void TileProvider::ensureBiomeChunk(const ChunkPosition &chunk_position) {
		biomeMap.try_emplace(chunk_position);
	}

	void TileProvider::ensurePathChunk(const ChunkPosition &chunk_position) {
		pathMap.try_emplace(chunk_position);
	}

	void TileProvider::validateLayer(Layer layer) const {
		if (layer == 0 || LAYER_COUNT < layer)
			throw std::out_of_range("Invalid layer: " + std::to_string(layer));
	}

	void to_json(nlohmann::json &json, const TileProvider &provider) {
		json.push_back(provider.tilesetID);

		nlohmann::json tile_array;
		for (Layer layer = 0; layer < LAYER_COUNT; ++layer)
			for (const auto &[position, chunk]: provider.chunkMaps[layer])
				tile_array.push_back(std::make_pair(std::make_tuple(layer, position.x, position.y), compress(std::span(chunk.data(), chunk.size()))));
		json.push_back(std::move(tile_array));

		nlohmann::json biome_array;
		for (const auto &[position, chunk]: provider.biomeMap)
			biome_array.push_back(std::make_pair(std::make_pair(position.x, position.y), compress(std::span(chunk.data(), chunk.size()))));
		json.push_back(std::move(biome_array));

		nlohmann::json path_array;
		for (const auto &[position, chunk]: provider.pathMap)
			path_array.push_back(std::make_pair(std::make_pair(position.x, position.y), compress(std::span(chunk.data(), chunk.size()))));
		json.push_back(std::move(path_array));
	}

	void from_json(const nlohmann::json &json, TileProvider &provider) {
		provider.tilesetID = json.at(0);

		for (const auto &item: json.at(1)) {
			const auto [layer, x, y] = item.at(0).get<std::tuple<Layer, int32_t, int32_t>>();
			const auto compressed = item.at(1).get<std::vector<uint8_t>>();
			provider.chunkMaps[layer][ChunkPosition{x, y}] = decompress16(std::span(compressed.data(), compressed.size()));
		}

		for (const auto &item: json.at(2)) {
			const auto [x, y] = item.at(1).get<std::pair<int32_t, int32_t>>();
			const auto compressed = item.at(1).get<std::vector<uint8_t>>();
			provider.biomeMap[ChunkPosition{x, y}] = decompress32(std::span(compressed.data(), compressed.size()));
		}

		for (const auto &item: json.at(2)) {
			const auto [x, y] = item.at(1).get<std::pair<int32_t, int32_t>>();
			const auto compressed = item.at(1).get<std::vector<uint8_t>>();
			provider.pathMap[ChunkPosition{x, y}] = decompress8(std::span(compressed.data(), compressed.size()));
		}
	}

	ChunkPosition getChunkPosition(Index row, Index column) {
		return {TileProvider::divide(column), TileProvider::divide(row)};
	}
}
