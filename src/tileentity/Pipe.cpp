#include "Log.h"
#include "Tileset.h"
#include "realm/Realm.h"
#include "tileentity/Pipe.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	Pipe::Pipe(Identifier tile_entity_id, Identifier corner_, Position position_):
		TileEntity("base:tile/missing"_id, std::move(tile_entity_id), position_, false),
		corner(std::move(corner_)) {}

	DirectionalContainer<std::shared_ptr<Pipe>> Pipe::getConnected() const {
		DirectionalContainer<std::shared_ptr<Pipe>> out;
		auto realm = getRealm();

		for (const Direction direction: directions.toVector())
			if (auto neighbor = realm->tileEntityAt(position + direction))
				if (auto neighbor_pipe = std::dynamic_pointer_cast<Pipe>(neighbor))
					if (neighbor_pipe->directions.has(flipDirection(direction)))
						out[direction] = neighbor_pipe;

		return out;
	}

	void Pipe::updateTileID() {
		RealmPtr realm = getRealm();
		Tileset &tileset = realm->getTileset();
		const auto march_index = directions.getMarchIndex();
		tileID = tileset[corner] + tileset.columnCount(realm->getGame()) * (march_index / 7) + march_index % 7;
	}

	void Pipe::render(SpriteRenderer &sprite_renderer) {
		RealmPtr realm = getRealm();
		Tileset &tileset = realm->getTileset();

		if (tileID == 0)
			updateTileID();

		if (extractorsCorner == static_cast<uint16_t>(-1))
			extractorsCorner = tileset["base:tile/extractor_"_id];

		const auto tilesize = tileset.getTileSize();
		const auto texture = tileset.getTexture(realm->getGame());
		auto x = (tileID % (*texture->width / tilesize)) * tilesize;
		auto y = (tileID / (*texture->width / tilesize)) * tilesize;
		sprite_renderer(*texture, {
			.x = static_cast<float>(position.column),
			.y = static_cast<float>(position.row),
			.x_offset = static_cast<float>(x) / 2.f,
			.y_offset = static_cast<float>(y) / 2.f,
			.size_x = static_cast<float>(tilesize),
			.size_y = static_cast<float>(tilesize),
		});

		const auto [extractors_x, extractors_y] = extractors.getOffsets();
		if (extractors_x != 0 || extractors_y != 0) {
			const TileID extractor_tile = extractorsCorner + extractors_x + extractors_y * tileset.columnCount(realm->getGame());
			x = (extractor_tile % (*texture->width / tilesize)) * tilesize;
			y = (extractor_tile / (*texture->width / tilesize)) * tilesize;
			sprite_renderer(*texture, {
				.x = static_cast<float>(position.column),
				.y = static_cast<float>(position.row),
				.x_offset = static_cast<float>(x) / 2.f,
				.y_offset = static_cast<float>(y) / 2.f,
				.size_x = static_cast<float>(tilesize),
				.size_y = static_cast<float>(tilesize),
			});
		}
	}

	void Pipe::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << directions;
		buffer << extractors;
		buffer << corner;
	}

	void Pipe::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		buffer >> directions;
		buffer >> static_cast<Directions &>(extractors);
		buffer >> corner;
		tileID = 0;
		extractorsCorner = -1;
	}

	void Pipe::setNetwork(const std::shared_ptr<PipeNetwork> &new_network) {
		weakNetwork = new_network;
		loaded = true;
	}

	std::shared_ptr<PipeNetwork> Pipe::getNetwork() const {
		return weakNetwork.lock();
	}
}
