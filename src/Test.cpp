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

		std::vector<uint8_t> test_vector {0x01, 0x02, 0x03, 0x04};

		std::map<std::string, std::vector<uint16_t>> test_map {
			{"Hello", {100, 65535, 0x42}}
		};

		std::string test_string = "Test!";

		buffer << test_vector << test_map << test_string;

		std::cout << buffer << '\n';
		const auto popped_vector = buffer.pop<decltype(test_vector)>();
		std::cout << buffer << '\n';

		std::cout << '{';
		for (const size_t item: popped_vector)
			std::cout << ' ' << std::hex << item << std::dec;
		std::cout << " }\n";

		const auto popped_map = buffer.pop<decltype(test_map)>();
		std::cout << buffer << '\n';

		for (const auto &[key, value]: popped_map) {
			std::cout << "- " << key << " => {";
			for (const auto &item: value)
				std::cout << ' ' << std::hex << item << std::dec;
			std::cout << " }\n";
		}

		const auto popped_string = buffer.pop<decltype(test_string)>();
		std::cout << buffer << '\n';
		std::cout << popped_string << '\n';
	}

	void test() {
		testBuffer();
	}
}
