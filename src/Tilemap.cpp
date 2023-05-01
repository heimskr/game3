#include <zstd.h>

#include "Tilemap.h"
#include "Tileset.h"
#include "container/Quadtree.h"
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
		if (tileset && tileset->hasName("base:tile/lava"_id)) {
			const auto lava_id = lavaID = (*tileset)["base:tile/lava"_id];
			lavaQuadtree = std::make_shared<Quadtree>(width, height, [this, lava_id](Index row, Index column) {
				return (*this)(column, row) == lava_id;
			});
		}
	}

	Tilemap::~Tilemap() = default;

	std::shared_ptr<Texture> Tilemap::getTexture(const Game &game) {
		if (texture)
			return texture;
		return texture = game.registry<TextureRegistry>()[textureName];
	}

	void Tilemap::set(Index x, Index y, TileID value) {
		set(x + y * width, value);
	}

	void Tilemap::set(const Position &position, TileID value) {
		set(position.column + position.row * width, value);
	}

	void Tilemap::set(Index index, TileID value) {
		auto &tile = tiles[index];
		if (lavaQuadtree) {
			if (value == lavaID && tile != lavaID)
				lavaQuadtree->add(index / width, index % width);
			else if (value != lavaID && tile == lavaID)
				lavaQuadtree->remove(index / width, index % width);
		}
		tile = value;
	}

	void Tilemap::reset(TileID value) {
		tiles.assign(tiles.size(), value);
		if (lavaQuadtree) {
			lavaQuadtree->reset();
			if (lavaID && *lavaID == value)
				lavaQuadtree->absorb(false);
		}
	}

	void to_json(nlohmann::json &json, const Tilemap &tilemap) {
		json["height"] = tilemap.height;
		json["setHeight"] = tilemap.setHeight;
		json["setWidth"] = tilemap.setWidth;
		json["tileSize"] = tilemap.tileSize;
		json["width"] = tilemap.width;
		json["tileset"] = tilemap.tileset->identifier;

		// TODO: fix endianness issues
		const auto tiles_size = tilemap.size() * sizeof(tilemap[0]);
		const auto buffer_size = ZSTD_compressBound(tiles_size);
		auto buffer = std::vector<uint8_t>(buffer_size);
		auto result = ZSTD_compress(&buffer[0], buffer_size, tilemap.data(), tiles_size, ZSTD_maxCLevel());
		if (ZSTD_isError(result))
			throw std::runtime_error("Couldn't compress tiles");
		buffer.resize(result);
		json["tiles"] = std::move(buffer);
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

		if (tilemap.lavaQuadtree)
			tilemap.lavaQuadtree->absorb();

		return tilemap;
	}
}
