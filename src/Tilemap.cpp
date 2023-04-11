#include <zstd.h>

#include "Tilemap.h"
#include "Tileset.h"
#include "game/Game.h"

namespace Game3 {
	Tilemap::Tilemap(int width_, int height_, int tile_size, int set_width, int set_height, std::shared_ptr<Tileset> tileset_):
	width(width_), height(height_), tileSize(tile_size), textureName(tileset_->getTextureName()), setWidth(set_width), setHeight(set_height), tileset(std::move(tileset_)) {
		tiles.resize(width * height);
	}

	Tilemap::Tilemap(int width_, int height_, int tile_size, std::shared_ptr<Tileset> tileset_):
	width(width_), height(height_), tileSize(tile_size), textureName(tileset_->getTextureName()), tileset(std::move(tileset_)) {
		tiles.resize(width * height);
	}

	void Tilemap::init(const Game &game) {
		getTexture(game)->init();
		setWidth = *texture->width;
		setHeight = *texture->height;
	}

	std::shared_ptr<Texture> Tilemap::getTexture(const Game &game) {
		if (texture)
			return texture;
		return texture = game.registry<TextureRegistry>()[textureName];
	}

	std::vector<Index> Tilemap::getLand(Index right_pad, Index bottom_pad) const {
		std::vector<Index> land_tiles;
		land_tiles.reserve(width * height);
		for (Index row = 0; row < height - bottom_pad; ++row)
			for (Index column = 0; column < width - right_pad; ++column)
				if (tileset->isLand(tiles[row * width + column]))
					land_tiles.push_back(row * width + column);
		return land_tiles;
	}

	nlohmann::json Tilemap::toJSON(const Game &game) {
		nlohmann::json json;
		json["height"] = height;
		json["setHeight"] = setHeight;
		json["setWidth"] = setWidth;
		json["tileSize"] = tileSize;
		json["width"] = width;
		json["tileset"] = tileset->identifier;

		// TODO: fix endianness issues
		const auto tiles_size = tiles.size() * sizeof(tiles[0]);
		const auto buffer_size = ZSTD_compressBound(tiles_size);
		auto buffer = std::vector<uint8_t>(buffer_size);
		auto result = ZSTD_compress(&buffer[0], buffer_size, tiles.data(), tiles_size, ZSTD_maxCLevel());
		if (ZSTD_isError(result))
			throw std::runtime_error("Couldn't compress tiles");
		buffer.resize(result);
		json["tiles"] = std::move(buffer);
		return json;
	}

	Tilemap Tilemap::fromJSON(const Game &game, const nlohmann::json &json) {
		auto tileset = game.registry<TilesetRegistry>()[json.at("tileset").get<Identifier>()];
		Tilemap tilemap(json.at("height"), json.at("width"), json.at("tileSize"), json.at("setWidth"), json.at("setHeight"), tileset);

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

		return tilemap;
	}
}
