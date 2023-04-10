#include "Tileset.h"
#include "realm/Realm.h"
#include "worldgen/Carpet.h"

namespace Game3::WorldGen {
	void generateCarpet(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, int padding) {
		const auto width  = realm->getWidth();
		const auto height = realm->getHeight();
		const int carpet_offset = 8 * (rng() % 3);
		const int carpet_padding = padding == -1? (rng() % 2) + 2 : padding;
		const auto &tileset = *realm->tilemap1->tileset;

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
				realm->setLayer1(row * width + column, carpet1c + carpet_offset);

		for (int row = carpet_padding + 1; row < height - carpet_padding - 1; ++row) {
			realm->setLayer1(row * width + carpet_padding, carpet1w + carpet_offset);
			realm->setLayer1((row + 1) * width - carpet_padding - 1, carpet1e + carpet_offset);
		}

		for (int column = carpet_padding + 1; column < width - carpet_padding - 1; ++column) {
			realm->setLayer1(carpet_padding * width + column, carpet1n + carpet_offset);
			realm->setLayer1((height - carpet_padding - 1) * width + column, carpet1s + carpet_offset);
		}

		realm->setLayer1(carpet_padding * width + carpet_padding, carpet1nw + carpet_offset);
		realm->setLayer1((carpet_padding + 1) * width - carpet_padding - 1, carpet1ne + carpet_offset);
		realm->setLayer1((height - carpet_padding - 1) * width + carpet_padding, carpet1sw + carpet_offset);
		realm->setLayer1((height - carpet_padding) * width - carpet_padding - 1, carpet1se + carpet_offset);
	}
}
