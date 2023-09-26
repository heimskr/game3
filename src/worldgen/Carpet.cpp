#include "graphics/Tileset.h"
#include "realm/Realm.h"
#include "worldgen/Carpet.h"

namespace Game3::WorldGen {
	void generateCarpet(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, Index width, Index height, int padding, Layer layer) {
		const int carpet_padding = padding < 0? std::uniform_int_distribution(2, 3)(rng) : padding;
		const auto &tileset = realm->getTileset();

		const std::string base = "base:tile/carpet" + std::to_string(std::uniform_int_distribution(1, 3)(rng)) + '_';

		const TileID carpet1c  = tileset[Identifier(base +  'c')];
		const TileID carpet1w  = tileset[Identifier(base +  'w')];
		const TileID carpet1e  = tileset[Identifier(base +  'e')];
		const TileID carpet1n  = tileset[Identifier(base +  'n')];
		const TileID carpet1s  = tileset[Identifier(base +  's')];
		const TileID carpet1nw = tileset[Identifier(base + "nw")];
		const TileID carpet1ne = tileset[Identifier(base + "ne")];
		const TileID carpet1sw = tileset[Identifier(base + "sw")];
		const TileID carpet1se = tileset[Identifier(base + "se")];

		for (int row = carpet_padding + 1; row < height - carpet_padding - 1; ++row)
			for (int column = carpet_padding + 1; column < width - carpet_padding - 1; ++column)
				realm->setTile(layer, {row, column}, carpet1c, false, true);

		for (int row = carpet_padding + 1; row < height - carpet_padding - 1; ++row) {
			realm->setTile(layer, {row, carpet_padding}, carpet1w, false, true);
			realm->setTile(layer, {row, width - carpet_padding - 1}, carpet1e, false, true);
		}

		for (int column = carpet_padding + 1; column < width - carpet_padding - 1; ++column) {
			realm->setTile(layer, {carpet_padding, column}, carpet1n, false, true);
			realm->setTile(layer, {height - carpet_padding - 1, column}, carpet1s, false, true);
		}

		realm->setTile(layer, {carpet_padding, carpet_padding}, carpet1nw, false, true);
		realm->setTile(layer, {carpet_padding, width - carpet_padding - 1}, carpet1ne, false, true);
		realm->setTile(layer, {height - carpet_padding - 1, carpet_padding}, carpet1sw, false, true);
		realm->setTile(layer, {height - carpet_padding - 1, width - carpet_padding - 1}, carpet1se, false, true);
	}
}
