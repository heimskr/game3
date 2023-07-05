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

	void Pipe::render(SpriteRenderer &sprite_renderer) {
		auto realm = getRealm();
		auto &tileset = realm->getTileset();
		if (tileID == 0)
			tileID = tileset[corner];
		const auto tilesize = tileset.getTileSize();
		const auto texture = tileset.getTexture(realm->getGame());
		const auto x = (tileID % (*texture->width / tilesize)) * tilesize;
		const auto y = (tileID / (*texture->width / tilesize)) * tilesize;
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
