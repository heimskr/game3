#include "Tileset.h"
#include "realm/Realm.h"
#include "worldgen/Carpet.h"

namespace Game3::WorldGen {
	void generateCarpet(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, Index width, Index height, int padding) {
		const int carpet_offset = 8 * (rng() % 3);
		const int carpet_padding = padding == -1? (rng() % 2) + 2 : padding;
		const auto &tileset = realm->getTileset();

		const TileID carpet1c  = tileset["base:tile/carpet1_c"];
		const TileID carpet1w  = tileset["base:tile/carpet1_w"];
		const TileID carpet1e  = tileset["base:tile/carpet1_e"];
		const TileID carpet1n  = tileset["base:tile/carpet1_n"];
		const TileID carpet1s  = tileset["base:tile/carpet1_s"];
		const TileID carpet1nw = tileset["base:tile/carpet1_nw"];
		const TileID carpet1ne = tileset["base:tile/carpet1_ne"];
		const TileID carpet1sw = tileset["base:tile/carpet1_sw"];
		const TileID carpet1se = tileset["base:tile/carpet1_se"];

		for (int row = carpet_padding + 1; row < height - carpet_padding - 1; ++row)
			for (int column = carpet_padding + 1; column < width - carpet_padding - 1; ++column)
				realm->setTile(1, {row, column}, carpet1c + carpet_offset);

		for (int row = carpet_padding + 1; row < height - carpet_padding - 1; ++row) {
			realm->setTile(1, {row, carpet_padding}, carpet1w + carpet_offset);
			realm->setTile(1, {row, width - carpet_padding - 1}, carpet1e + carpet_offset);
		}

		for (int column = carpet_padding + 1; column < width - carpet_padding - 1; ++column) {
			realm->setTile(1, {carpet_padding, column}, carpet1n + carpet_offset);
			realm->setTile(1, {height - carpet_padding - 1, column}, carpet1s + carpet_offset);
		}

		realm->setTile(1, {carpet_padding, carpet_padding}, carpet1nw + carpet_offset);
		realm->setTile(1, {carpet_padding, width - carpet_padding - 1}, carpet1ne + carpet_offset);
		realm->setTile(1, {height - carpet_padding - 1, carpet_padding}, carpet1sw + carpet_offset);
		realm->setTile(1, {height - carpet_padding - 1, width - carpet_padding - 1}, carpet1se + carpet_offset);
	}
}
