#include <deque>

#include <zstd.h>

#include "Tilemap.h"
#include "Tiles.h"

namespace Game3 {
	std::vector<Index> Tilemap::getLand(RealmType type, size_t right_pad, size_t bottom_pad) const {
		std::vector<Index> land_tiles;
		land_tiles.reserve(width * height);
		const auto &tileset = *tileSets.at(type);
		for (size_t row = 0; row < height - bottom_pad; ++row)
			for (size_t column = 0; column < width - right_pad; ++column)
				if (tileset.isLand(tiles[row * width + column]))
					land_tiles.push_back(row * width + column);
		return land_tiles;
	}

	void to_json(nlohmann::json &json, const Tilemap &tilemap) {
		json["height"] = tilemap.height;
		json["setHeight"] = tilemap.setHeight;
		json["setWidth"] = tilemap.setWidth;
		json["texture"] = tilemap.texture;
		json["tileSize"] = tilemap.tileSize;
		json["width"] = tilemap.width;

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

	void from_json(const nlohmann::json &json, Tilemap &tilemap) {
		tilemap.height = json.at("height");
		tilemap.setHeight = json.at("setHeight");
		tilemap.setWidth = json.at("setWidth");
		tilemap.texture = json.at("texture");
		tilemap.tileSize = json.at("tileSize");
		tilemap.width = json.at("width");

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
