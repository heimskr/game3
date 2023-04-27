#include "game/TileProvider.h"
#include "util/Util.h"

namespace Game3 {
	TileID Chunk::operator()(size_t row, size_t column) const {
		return (*this)[row * CHUNK_SIZE + column];
	}

	TileID & Chunk::operator()(size_t row, size_t column) {
		return (*this)[row * CHUNK_SIZE + column];
	}

	/** Given chunk_size = 32: [0, 32) -> 0, [32, 64) -> 1, [-32, 0) -> -1 */
	template <typename I, typename O = int32_t>
	static O divide(I value, I chunk_size = static_cast<I>(CHUNK_SIZE)) {
		if (0 <= value)
			return static_cast<O>(value / chunk_size);
		return static_cast<O>(-updiv(-value, chunk_size));
	}

	template <typename I, typename O = int32_t>
	static O remainder(I value, I chunk_size = static_cast<I>(CHUNK_SIZE)) {
		if (0 <= value)
			return static_cast<O>(value % chunk_size);
		return static_cast<O>((chunk_size - (-value % chunk_size)) % chunk_size);
	}

	TileID TileProvider::copyTile(Layer layer, Index row, Index column, Mode mode) const {
		validateLayer(layer);

		const auto &map = chunkMaps[layer - 1];
		const ChunkPosition chunk_position {divide(column), divide(row)};

		if (auto iter = map.find(chunk_position); iter != map.end())
			return iter->second(remainder(row), remainder(column));

		if (mode == Mode::ReturnEmpty)
			return 0;

		throw std::out_of_range("Couldn't copy tile (" + std::to_string(column) + ", " + std::to_string(row) + ')');
	}

	TileID & TileProvider::findTile(Layer layer, Index row, Index column, Mode mode) {
		validateLayer(layer);

		auto &map = chunkMaps[layer - 1];
		const ChunkPosition chunk_position {divide(column), divide(row)};

		if (auto iter = map.find(chunk_position); iter != map.end())
			return iter->second(remainder(row), remainder(column));

		if (mode == Mode::Create)
			return map[chunk_position](remainder(row), remainder(column)) = 0;

		throw std::out_of_range("Couldn't find tile (" + std::to_string(column) + ", " + std::to_string(row) + ')');
	}

	const Chunk & TileProvider::getChunk(Layer layer, const ChunkPosition &chunk_position) const {
		validateLayer(layer);

		if (auto iter = chunkMaps[layer - 1].find(chunk_position); iter != chunkMaps[layer - 1].end())
			return iter->second;

		throw std::runtime_error("Couldn't find chunk at position " + static_cast<std::string>(chunk_position));
	}

	Chunk & TileProvider::getChunk(Layer layer, const ChunkPosition &chunk_position) {
		validateLayer(layer);
		return chunkMaps[layer - 1][chunk_position];
	}

	void TileProvider::validateLayer(Layer layer) const {
		if (layer == 0 || LAYER_COUNT < layer)
			throw std::out_of_range("Invalid layer: " + std::to_string(layer));
	}
}
