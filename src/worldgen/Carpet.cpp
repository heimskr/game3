#include "Tiles.h"
#include "realm/Realm.h"
#include "worldgen/Carpet.h"

namespace Game3::WorldGen {
	void generateCarpet(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng) {
		const auto width  = realm->getWidth();
		const auto height = realm->getHeight();
		const int carpet_offset = 8 * (rng() % 3);
		const int carpet_padding = (rng() % 2) + 2;
		for (int row = carpet_padding + 1; row < height - carpet_padding - 1; ++row)
			for (int column = carpet_padding + 1; column < width - carpet_padding - 1; ++column)
				realm->setLayer1(row * width + column, HouseTiles::CARPET_C + carpet_offset);
		for (int row = carpet_padding + 1; row < height - carpet_padding - 1; ++row) {
			realm->setLayer1(row * width + carpet_padding, HouseTiles::CARPET_W + carpet_offset);
			realm->setLayer1((row + 1) * width - carpet_padding - 1, HouseTiles::CARPET_E + carpet_offset);
		}
		for (int column = carpet_padding + 1; column < width - carpet_padding - 1; ++column) {
			realm->setLayer1(carpet_padding * width + column, HouseTiles::CARPET_N + carpet_offset);
			realm->setLayer1((height - carpet_padding - 1) * width + column, HouseTiles::CARPET_S + carpet_offset);
		}
		realm->setLayer1(carpet_padding * width + carpet_padding, HouseTiles::CARPET_NW + carpet_offset);
		realm->setLayer1((carpet_padding + 1) * width - carpet_padding - 1, HouseTiles::CARPET_NE + carpet_offset);
		realm->setLayer1((height - carpet_padding - 1) * width + carpet_padding, HouseTiles::CARPET_SW + carpet_offset);
		realm->setLayer1((height - carpet_padding) * width - carpet_padding - 1, HouseTiles::CARPET_SE + carpet_offset);
	}
}
