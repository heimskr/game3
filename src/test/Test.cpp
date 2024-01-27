#include "Log.h"
#include "container/Quadtree.h"
#include "entity/ServerPlayer.h"
#include "game/TileProvider.h"
#include "net/Buffer.h"
#include "threading/LockableSharedPtr.h"
#include "threading/LockableWeakPtr.h"
#include "util/Timer.h"

#include <array>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include <nlohmann/json.hpp>

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

		auto visit = [](const Box &) {
			// std::cout << box << '\n';
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

		std::cout << std::format("{}\n", buffer);
		decltype(test_vector) popped_vector;
		buffer >> popped_vector;
		std::cout << std::format("{}\n", buffer);

		std::cout << '{';
		for (const size_t item: popped_vector)
			std::cout << ' ' << std::hex << item << std::dec;
		std::cout << " }\n";

		decltype(test_map) popped_map;
		buffer >> popped_map;
		std::cout << std::format("{}\n", buffer);

		for (const auto &[key, value]: popped_map) {
			std::cout << "- " << key << " => {";
			for (const auto &item: value)
				std::cout << ' ' << std::hex << item << std::dec;
			std::cout << " }\n";
		}

		decltype(test_string1) popped_string1;
		buffer >> popped_string1;
		std::cout << std::format("{}\n", buffer);
		std::cout << popped_string1 << '\n';

		decltype(test_string2) popped_string2;
		buffer >> popped_string2;
		std::cout << std::format("{}\n", buffer);
		std::cout << popped_string2 << '\n';

		decltype(test_string3) popped_string3;
		buffer >> popped_string3;
		std::cout << std::format("{}\n", buffer);
		std::cout << '[' << popped_string3 << "]\n";

		uint8_t popped8;
		buffer >> popped8;
		std::cout << std::format("{}\n", buffer);
		std::cout << "0x" << std::hex << popped8 << std::dec << '\n';

		uint16_t popped16;
		buffer >> popped16;
		std::cout << std::format("{}\n", buffer);
		std::cout << "0x" << std::hex << popped16 << std::dec << '\n';

		uint32_t popped32;
		buffer >> popped32;
		std::cout << std::format("{}\n", buffer);
		std::cout << "0x" << std::hex << popped32 << std::dec << '\n';

		uint64_t popped64;
		buffer >> popped64;
		std::cout << std::format("{}\n", buffer);
		std::cout << "0x" << std::hex << popped64 << std::dec << '\n';

		int32_t popped32i;
		buffer >> popped32i;
		std::cout << std::format("{}\n", buffer);
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

	void testLockableWeakPtr() {
		auto shared = std::make_shared<int>(42);
		LockableWeakPtr<int> lwp = shared;

		constexpr size_t sum_count = 1'000'000;

		auto lambda = [&] {
			size_t sum = 0;
			for (size_t i = 0; i < sum_count; ++i) {
				sum += lwp.use_count();
				lwp.reset();
			}
			INFO_("sum[" << sum << ']');
		};

		std::thread t1 = std::thread(lambda);
		std::thread t2 = std::thread(lambda);

		t1.join();
		t2.join();
	}

	void testGetFacing() {
		Position base(0, 0);
		auto check = [&base](Index row, Index column, Direction expected) {
			const Direction facing = base.getFacing({row, column});
			if (facing == expected)
				SUCCESS("({}, {}) → {}", row, column, facing);
			else
				ERROR("({}, {}) → {}, expected {}", row, column, facing, expected);
		};

		check( 0,  0, Direction::Invalid);
		check( 0, -1, Direction::Left);
		check( 0, -2, Direction::Left);
		check(-1, -1, Direction::Left);
		check(-2, -1, Direction::Up);
		check( 2, -1, Direction::Down);
		check( 0,  1, Direction::Right);
		check( 0,  2, Direction::Right);
		check(-1,  1, Direction::Right);
		check(-2,  1, Direction::Up);
		check( 2,  1, Direction::Down);
		check( 1, -1, Direction::Left);
		check( 1,  1, Direction::Right);
		check( 1,  0, Direction::Down);
		check(-1,  0, Direction::Up);
	}

	void test() {
		// testPlayerJSON();
		// testLockableWeakPtr();
		// LockableSharedPtr<int> lsp = std::make_shared<int>(42);
		testGetFacing();
	}
}
