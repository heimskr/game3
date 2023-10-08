#pragma once

// MIT License

// Copyright (c) 2019 Tobias Brückner

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <array>
#include <random>

namespace Game3 {
	enum class MazeWall {North = 0b1000, East = 0b0100, South = 0b0010, West = 0b0001};

	struct MazeSize {
		size_t width  = 0;
		size_t height = 0;
	};

	struct MazeCoords {
		ssize_t x, y;

		constexpr MazeCoords(ssize_t point_x, ssize_t point_y):
			x(point_x), y(point_y) {}

		[[nodiscard]]
		constexpr bool operator==(const MazeCoords &other) const {
			return this == &other || (x == other.x && y == other.y);
		}
	};

	enum class MazeDirection {North = 0, East, South, West};

	constexpr inline uint8_t operator|(const MazeWall lhs, const MazeWall rhs) {
		return static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs);
	}

	constexpr inline uint8_t operator|(const uint8_t lhs, const MazeWall rhs) {
		return lhs | static_cast<uint8_t>(rhs);
	}

	class Mazer {
		public:
			class Node {
				public:
					constexpr Node():
						node_(0) {}

					constexpr explicit Node(const uint8_t bitmask):
						node_(bitmask) { }

					constexpr explicit Node(const MazeWall wall):
						node_(static_cast<uint8_t>(wall)) {}

					[[nodiscard]]
					constexpr static Node withAllWalls() {
						return Node{MazeWall::North | MazeWall::East | MazeWall::South | MazeWall::West};
					}

					[[nodiscard]]
					constexpr bool hasWall(const MazeWall wall) const {
						return (node_ & static_cast<uint8_t>(wall)) == static_cast<uint8_t>(wall);
					}

					constexpr void setWall(const uint8_t bitmask) {
						node_ |= bitmask;
					}

					constexpr void setWall(const MazeWall wall) {
						setWall(static_cast<uint8_t>(wall));
					}

					constexpr void clearWall(const MazeWall wall) {
						node_ &= ~static_cast<uint8_t>(wall);
					}

					[[nodiscard]]
					constexpr bool visited() const {
						return (node_ & 0b10000) == 0b10000;
					}

					constexpr void setVisited() {
						node_ |= 0b10000;
					}

					[[nodiscard]]
					constexpr uint8_t value() const {
						return node_ & 0b1111;
					}

				private:
					uint8_t node_;
			};

			Mazer(MazeSize size_, uint_fast32_t seed_, const MazeCoords &starting_point):
			size(size_),
			nodes(static_cast<size_t>(size.width * size.height), Node::withAllWalls()),
			seed(0 < seed_? static_cast<uint_fast32_t>(seed_) : randomDevice()),
			randomGenerator(randomDevice()),
			randomDistribution{0, 23} {
				generate(starting_point);
			}

			[[nodiscard]]
			MazeSize getSize() const {
				return size;
			}

			[[nodiscard]]
			bool validCoords(const MazeCoords &coords) const {
				return 0 <= coords.x && coords.x < static_cast<ssize_t>(size.width) && 0 <= coords.y && coords.y < static_cast<ssize_t>(size.height);
			}

			[[nodiscard]]
			Node & node(const MazeCoords &coords) {
				return nodes[coords.y * size.width + coords.x];
			}

			[[nodiscard]]
			const Node & node(const MazeCoords &coords) const {
				return nodes[coords.y * size.width + coords.x];
			}

			[[nodiscard]]
			static MazeCoords coordsInDirection(const MazeCoords &coords, MazeDirection dir) {
				constexpr static std::array direction_coord_offsets{MazeCoords{0, -1}, MazeCoords{1, 0}, MazeCoords{0, 1}, MazeCoords{-1, 0}};
				const MazeCoords &offset = direction_coord_offsets[static_cast<size_t>(dir)];
				return MazeCoords{coords.x + offset.x, coords.y + offset.y};
			}

			[[nodiscard]]
			static MazeWall wallInDirection(MazeDirection dir) {
				constexpr static std::array walls{MazeWall::North, MazeWall::East, MazeWall::South, MazeWall::West};
				return walls[static_cast<std::size_t>(dir)];
			}

			[[nodiscard]]
			static MazeDirection oppositeDirection(MazeDirection dir) {
				constexpr static std::array directions{MazeDirection::South, MazeDirection::West, MazeDirection::North, MazeDirection::East};
				return directions[static_cast<std::size_t>(dir)];
			}

			[[nodiscard]]
			const std::array<MazeDirection, 4> & randomDirections() {
				constexpr static std::array all_possible_random_directions{
					std::array{MazeDirection::North, MazeDirection::East,  MazeDirection::South, MazeDirection::West },
					std::array{MazeDirection::North, MazeDirection::East,  MazeDirection::West,  MazeDirection::South},
					std::array{MazeDirection::North, MazeDirection::South, MazeDirection::East,  MazeDirection::West },
					std::array{MazeDirection::North, MazeDirection::South, MazeDirection::West,  MazeDirection::East },
					std::array{MazeDirection::North, MazeDirection::West,  MazeDirection::East,  MazeDirection::South},
					std::array{MazeDirection::North, MazeDirection::West,  MazeDirection::South, MazeDirection::East },
					std::array{MazeDirection::East,  MazeDirection::North, MazeDirection::South, MazeDirection::West },
					std::array{MazeDirection::East,  MazeDirection::North, MazeDirection::West,  MazeDirection::South},
					std::array{MazeDirection::East,  MazeDirection::South, MazeDirection::North, MazeDirection::West },
					std::array{MazeDirection::East,  MazeDirection::South, MazeDirection::West,  MazeDirection::North},
					std::array{MazeDirection::East,  MazeDirection::West,  MazeDirection::North, MazeDirection::South},
					std::array{MazeDirection::East,  MazeDirection::West,  MazeDirection::South, MazeDirection::North},
					std::array{MazeDirection::South, MazeDirection::North, MazeDirection::East,  MazeDirection::West },
					std::array{MazeDirection::South, MazeDirection::North, MazeDirection::West,  MazeDirection::East },
					std::array{MazeDirection::South, MazeDirection::East,  MazeDirection::North, MazeDirection::West },
					std::array{MazeDirection::South, MazeDirection::East,  MazeDirection::West,  MazeDirection::North},
					std::array{MazeDirection::South, MazeDirection::West,  MazeDirection::North, MazeDirection::East },
					std::array{MazeDirection::South, MazeDirection::West,  MazeDirection::East,  MazeDirection::North},
					std::array{MazeDirection::West,  MazeDirection::North, MazeDirection::East,  MazeDirection::South},
					std::array{MazeDirection::West,  MazeDirection::North, MazeDirection::South, MazeDirection::East },
					std::array{MazeDirection::West,  MazeDirection::East,  MazeDirection::North, MazeDirection::South},
					std::array{MazeDirection::West,  MazeDirection::East,  MazeDirection::South, MazeDirection::North},
					std::array{MazeDirection::West,  MazeDirection::South, MazeDirection::North, MazeDirection::East },
					std::array{MazeDirection::West,  MazeDirection::South, MazeDirection::East,  MazeDirection::North}
				};

				return all_possible_random_directions[randomDistribution(randomGenerator)];
			}

			[[nodiscard]]
			uint_fast32_t getSeed() const {
				return seed;
			}

			void clearWalls(const MazeCoords &from, const MazeCoords &to, MazeDirection dir) {
				const MazeWall orig_wall = wallInDirection(dir);
				const MazeWall dest_wall = wallInDirection(oppositeDirection(dir));
				node(from).clearWall(orig_wall);
				node(to).clearWall(dest_wall);
			}

		private:
			class StackNode {
				public:
					StackNode(const MazeCoords &coords_, const std::array<MazeDirection, 4> &check_directions):
					coords(coords_),
					checkDirections(check_directions) {}

					[[nodiscard]]
					const MazeCoords & getCoords() const {
						return coords;
					}

					[[nodiscard]] bool hasCheckedAllDirections() const {
						return 4 <= index;
					}

					[[nodiscard]] MazeDirection nextDirection() {
						return checkDirections[index++];
					}

				private:
					MazeCoords coords;
					std::array<MazeDirection, 4> checkDirections;
					size_t index = 0;
			};

		public:
			void generate(const MazeCoords &starting_point) {
				std::vector<StackNode> stack;
				randomGenerator.seed(seed);
				node(starting_point).setVisited();
				stack.emplace_back(starting_point, randomDirections());

				while (!stack.empty()) {
					StackNode &current_node = stack.back();

					if (current_node.hasCheckedAllDirections()) {
						stack.pop_back();
						continue;
					}

					bool keep_checking = true;

					while (keep_checking && !current_node.hasCheckedAllDirections()) {
						const MazeDirection dir = current_node.nextDirection();
						const MazeCoords next_coords = coordsInDirection(current_node.getCoords(), dir);

						if (validCoords(next_coords)) {
							Node &next_node = node(next_coords);

							if (!next_node.visited()) {
								clearWalls(current_node.getCoords(), next_coords, dir);
								next_node.setVisited();

								stack.emplace_back(next_coords, randomDirections());
								keep_checking = false;
							}
						}
					}
				}
			}

			std::vector<uint8_t> getRaw() const {
				std::vector<uint8_t> out;

				write([&out](const auto item) {
					out.emplace_back(item);
				});

				return out;
			}

			std::vector<std::vector<uint8_t>> getRows(bool chomp_edge) const {
				std::vector<std::vector<uint8_t>> out;
				std::vector<uint8_t> row;

				write([&](const auto item) {
					row.emplace_back(item);
					if (row.size() == 2 * size.width + 1) {
						if (chomp_edge)
							row.pop_back();
						out.push_back(std::move(row));
					}
				});

				if (chomp_edge)
					out.pop_back();

				return out;
			}

		private:
			MazeSize size;
			std::vector<Node> nodes;

			std::random_device randomDevice;
			uint_fast32_t seed;
			std::mt19937 randomGenerator;
			std::uniform_int_distribution<int> randomDistribution;

			void write(const auto &writer) const {
				constexpr static uint8_t black = 0;
				constexpr static uint8_t white = 1;

				for (ssize_t row = 0; row < static_cast<ssize_t>(size.height); ++row) {
					for (ssize_t col = 0; col < static_cast<ssize_t>(size.width); ++col) {
						if (node({col, row}).hasWall(MazeWall::North)) {
							writer(white);
							writer(white);
						} else {
							writer(white);
							writer(black);
						}
					}

					writer(white);

					for (ssize_t col = 0; col < static_cast<ssize_t>(size.width); ++col) {
						if (node({col, row}).hasWall(MazeWall::West)) {
							writer(white);
							writer(black);
						} else {
							writer(black);
							writer(black);
						}
					}

					writer(white);
				}

				for (size_t i = 0; i < 2 * size.width + 1; ++i)
					writer(white);
			}
	};
}
