#include <array>
#include <iostream>

// #include "game/Game.h"
#include "container/Quadtree.h"
#include "game/TileProvider.h"
#include "net/Buffer.h"

namespace Game3 {
	void testQuadtree() {
		std::array<std::array<bool, 8>, 8> tiles {
			std::array<bool, 8> {1, 1, 0, 0, 1, 1, 0, 0},
			std::array<bool, 8> {1, 1, 0, 0, 1, 1, 0, 0},
			std::array<bool, 8> {0, 0, 1, 1, 0, 1, 0, 0},
			std::array<bool, 8> {0, 0, 1, 1, 1, 0, 0, 0},
			std::array<bool, 8> {1, 1, 1, 1, 0, 0, 0, 0},
			std::array<bool, 8> {1, 1, 1, 1, 0, 1, 1, 0},
			std::array<bool, 8> {1, 1, 1, 1, 0, 1, 1, 0},
			std::array<bool, 8> {1, 1, 1, 1, 0, 0, 0, 0},
		};

		constexpr size_t W = tiles.size();
		constexpr size_t H = tiles[0].size();

		Quadtree tree(W, H, [&tiles](Index row, Index column) {
			return tiles[row][column];
		});

		auto visit = [](const Box &box) {
			std::cout << box << '\n';
			return false;
		};

		tree.iterateFull(visit);

		tree.remove(0, 0);
		std::cout << '\n';

		tree.iterateFull(visit);
	}

	void testTileProvider() {
		for (int i = -128; i <= 64; ++i)
			std::cout << i << " -> " << TileProvider::divide(i) << ", " << TileProvider::remainder(i) << "\n";
	}

	void testBuffer() {
		Buffer buffer;
		buffer << std::map<std::string, std::vector<uint16_t>> {
			{"Hello", {100, 65535, 0x42}}
		};
		std::cout << buffer << '\n';
	}

	void test() {
		testBuffer();
	}
}
