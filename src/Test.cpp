#include <array>
#include <iostream>

// #include "game/Game.h"
#include "container/Quadtree.h"

namespace Game3 {
	constexpr static int WIDTH = 1000;

	int getIndex(int r, int c) {
		return r * WIDTH + c;
	}

	void test() {
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
}
