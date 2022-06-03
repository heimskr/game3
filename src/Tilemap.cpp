#include <zstd.h>

#include "Tilemap.h"

namespace Game3 {
	void to_json(nlohmann::json &json, const Tilemap &tilemap) {
		json["height"] = tilemap.height;
		json["setHeight"] = tilemap.setHeight;
		json["setWidth"] = tilemap.setWidth;
		json["texture"] = tilemap.texture;
		json["tileSize"] = tilemap.tileSize;
		json["width"] = tilemap.width;

		const auto tiles_size = tilemap.tiles.size() * sizeof(tilemap.tiles[0]);
		const auto buffer_size = ZSTD_compressBound(tiles_size);
		auto buffer = std::vector<uint8_t>(buffer_size);
		auto result = ZSTD_compress(&buffer[0], buffer_size, tilemap.tiles.data(), tiles_size, ZSTD_maxCLevel());
		if (ZSTD_isError(result))
			throw std::runtime_error("Couldn't compress tileset");
		buffer.resize(result);
		json["tiles"] = std::move(buffer);
	}
}