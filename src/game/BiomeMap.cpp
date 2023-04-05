#include <zstd.h>

#include "game/BiomeMap.h"

namespace Game3 {
	void to_json(nlohmann::json &json, const BiomeMap &tilemap) {
		json["height"] = tilemap.height;
		json["width"]  = tilemap.width;

		// TODO: fix endianness issues
		const auto tiles_size = tilemap.tiles.size() * sizeof(tilemap.tiles[0]);
		const auto buffer_size = ZSTD_compressBound(tiles_size);
		auto buffer = std::vector<uint8_t>(buffer_size);
		auto result = ZSTD_compress(&buffer[0], buffer_size, tilemap.tiles.data(), tiles_size, ZSTD_maxCLevel());
		if (ZSTD_isError(result))
			throw std::runtime_error("Couldn't compress tiles");
		buffer.resize(result);
		json["tiles"] = std::move(buffer);
	}

	void from_json(const nlohmann::json &json, BiomeMap &tilemap) {
		tilemap.height = json.at("height");
		tilemap.width  = json.at("width");

		// TODO: fix endianness issues
		tilemap.tiles.clear();
		const size_t out_size = ZSTD_DStreamOutSize();
		std::vector<uint8_t> out_buffer(out_size);
		std::vector<uint8_t> bytes = json.at("tiles");

		auto context = std::unique_ptr<ZSTD_DCtx, size_t(*)(ZSTD_DCtx *)>(ZSTD_createDCtx(), ZSTD_freeDCtx);
		auto stream = std::unique_ptr<ZSTD_DStream, size_t(*)(ZSTD_DStream *)>(ZSTD_createDStream(), ZSTD_freeDStream);

		ZSTD_inBuffer input {bytes.data(), bytes.size(), 0};

		size_t last_result = 0;

		while (input.pos < input.size) {
			ZSTD_outBuffer output {&out_buffer[0], out_size, 0};
			const size_t result = ZSTD_decompressStream(stream.get(), &output, &input);
			if (ZSTD_isError(result))
				throw std::runtime_error("Couldn't decompress tiles");
			last_result = result;
			const TileID *raw_tiles = reinterpret_cast<const TileID *>(out_buffer.data());
			tilemap.tiles.insert(tilemap.tiles.end(), raw_tiles, raw_tiles + output.pos / sizeof(TileID));
		}

		if (last_result != 0)
			throw std::runtime_error("Reached end of tile input without finishing decompression");
	}
}
