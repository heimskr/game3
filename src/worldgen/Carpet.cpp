#include "graphics/Tileset.h"
#include "realm/Realm.h"
#include "worldgen/Carpet.h"

namespace Game3::WorldGen {
	void generateCarpet(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, Index width, Index height, int padding, Layer layer) {
		auto guard = realm->guardGeneration();
		const int carpet_padding = padding < 0? std::uniform_int_distribution(2, 3)(rng) : padding;

		constexpr static std::array<std::string, 3> colors{"blue", "red", "purple"};
		const Identifier carpet("base:tile/" + colors[std::uniform_int_distribution(0, 2)(rng)] + "_carpet");

		for (int row = carpet_padding + 1; row < height - carpet_padding - 1; ++row)
			for (int column = carpet_padding + 1; column < width - carpet_padding - 1; ++column)
				realm->setTile(layer, {row, column}, carpet);

		for (int row = carpet_padding + 1; row < height - carpet_padding - 1; ++row) {
			realm->setTile(layer, {row, carpet_padding}, carpet);
			realm->setTile(layer, {row, width - carpet_padding - 1}, carpet);
		}

		for (int column = carpet_padding + 1; column < width - carpet_padding - 1; ++column) {
			realm->setTile(layer, {carpet_padding, column}, carpet);
			realm->setTile(layer, {height - carpet_padding - 1, column}, carpet);
		}

		realm->setTile(layer, {carpet_padding, carpet_padding}, carpet);
		realm->setTile(layer, {carpet_padding, width - carpet_padding - 1}, carpet);
		realm->setTile(layer, {height - carpet_padding - 1, carpet_padding}, carpet);
		realm->setTile(layer, {height - carpet_padding - 1, width - carpet_padding - 1}, carpet);
	}
}
