#include "Log.h"
#include "realm/Realm.h"
#include "algorithm/AStar.h"

#include <algorithm>
#include <queue>
#include <vector>

namespace Game3 {
	template<typename T, typename Priority>
	struct PriorityQueue {
		using Element = std::pair<Priority, T>;
		std::priority_queue<Element, std::vector<Element>, std::greater<Element>> elements;

		inline bool empty() const {
			return elements.empty();
		}

		inline void put(T item, Priority priority) {
			elements.emplace(priority, item);
		}

		T get() {
			T best_item = elements.top().second;
			elements.pop();
			return best_item;
		}
	};

	namespace {
		inline size_t heuristic(const Position &a, const Position &b) {
			return std::abs(a.row - b.row) + std::abs(a.column - b.column);
		}

		inline void getNeighbors(const std::shared_ptr<Realm> &realm, const Position &position, std::vector<Position> &next) {
			next.clear();

			auto check = [&](const Position &pos) {
				if (auto state = realm->tileProvider.copyPathState(pos); state && *state != 0 && !realm->hasFluid(pos))
					next.emplace_back(pos);
			};

			check({position.row - 1, position.column});
			check({position.row, position.column - 1});
			check({position.row + 1, position.column});
			check({position.row, position.column + 1});
		}
	}

	// Credit: https://www.redblobgames.com/pathfinding/a-star/implementation.html#cplusplus
	bool simpleAStar(const std::shared_ptr<Realm> &realm, const Position &start, const Position &goal, std::vector<Position> &path, size_t loop_max) {
		std::unordered_map<Position, Position> moves;
		std::unordered_map<Position, size_t> costs;
		PriorityQueue<Position, size_t> frontier;
		frontier.put(start, 0);

		moves[start] = start;
		costs[start] = 0;

		std::vector<Position> next_positions;
		next_positions.reserve(4);

		for (size_t loops = 0; loops < loop_max && !frontier.empty(); ++loops) {
			Position current = frontier.get();
			if (current == goal) {
				path.clear();
				path.push_back(goal);
				auto iter = moves.find(goal);
				while (iter != moves.end()) {
					path.push_back(iter->second);
					if (iter->second == start)
						break;
					iter = moves.find(iter->second);
				}
				std::reverse(path.begin(), path.end());
				return true;
			}

			getNeighbors(realm, current, next_positions);

			for (const Position &next: next_positions) {
				const auto new_cost = costs[current] + 1;
				if (!costs.contains(next) || new_cost < costs[next]) {
					costs[next] = new_cost;
					frontier.put(next, new_cost + heuristic(next, goal));
					moves[next] = current;
				}
			}
		}

		return false;
	}
}