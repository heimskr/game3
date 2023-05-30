#include <array>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

// #include "game/Game.h"
#include "container/Quadtree.h"
#include "game/TileProvider.h"
#include "net/Buffer.h"
#include "util/Timer.h"
#include "entity/ServerPlayer.h"

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

		std::string test_string1 = "Test!";
		std::string test_string2 = "A bit of a longer test.";
		std::string test_string3;

		buffer << test_vector << test_map << test_string1 << test_string2 << test_string3;
		buffer << static_cast<uint8_t>(0x10) << static_cast<uint16_t>(0x2030) << static_cast<uint32_t>(0x40506070) << static_cast<uint64_t>(0x8090a0b0c0d0e0f0);
		buffer << -100000;

		std::cout << buffer << '\n';
		decltype(test_vector) popped_vector;
		buffer >> popped_vector;
		std::cout << buffer << '\n';

		std::cout << '{';
		for (const size_t item: popped_vector)
			std::cout << ' ' << std::hex << item << std::dec;
		std::cout << " }\n";

		decltype(test_map) popped_map;
		buffer >> popped_map;
		std::cout << buffer << '\n';

		for (const auto &[key, value]: popped_map) {
			std::cout << "- " << key << " => {";
			for (const auto &item: value)
				std::cout << ' ' << std::hex << item << std::dec;
			std::cout << " }\n";
		}

		decltype(test_string1) popped_string1;
		buffer >> popped_string1;
		std::cout << buffer << '\n';
		std::cout << popped_string1 << '\n';

		decltype(test_string2) popped_string2;
		buffer >> popped_string2;
		std::cout << buffer << '\n';
		std::cout << popped_string2 << '\n';

		decltype(test_string3) popped_string3;
		buffer >> popped_string3;
		std::cout << buffer << '\n';
		std::cout << '[' << popped_string3 << "]\n";

		uint8_t popped8;
		buffer >> popped8;
		std::cout << buffer << '\n';
		std::cout << "0x" << std::hex << popped8 << std::dec << '\n';

		uint16_t popped16;
		buffer >> popped16;
		std::cout << buffer << '\n';
		std::cout << "0x" << std::hex << popped16 << std::dec << '\n';

		uint32_t popped32;
		buffer >> popped32;
		std::cout << buffer << '\n';
		std::cout << "0x" << std::hex << popped32 << std::dec << '\n';

		uint64_t popped64;
		buffer >> popped64;
		std::cout << buffer << '\n';
		std::cout << "0x" << std::hex << popped64 << std::dec << '\n';

		int32_t popped32i;
		buffer >> popped32i;
		std::cout << buffer << '\n';
		std::cout << popped32i << '\n';
	}

	void testIteration() {
		Timer::clear();

		std::unordered_set<size_t> set;
		for (size_t i = 0; i < 100'000'000; ++i)
			set.insert(i);

		size_t sum = 0;

		Timer set_timer("std::unordered_set");
		for (const auto &item: set)
			sum += item;
		set_timer.stop();

		std::cout << "Set sum: " << sum << '\n';
		sum = 0;

		std::unordered_map<size_t, size_t> map;
		for (size_t i = 0; i < 100'000'000; ++i)
			map.emplace(i, i);

		Timer map_timer("std::unordered_map");
		for (const auto &[key, value]: map)
			sum += value;
		map_timer.stop();

		std::cout << "Map sum: " << sum << '\n';

		Timer::summary();
	}

	void testPlayerJSON() {
		auto player = Entity::create<ServerPlayer>();
		std::cout << nlohmann::json(*player).dump() << '\n';
	}

	void test() {
		testPlayerJSON();
	}
}
