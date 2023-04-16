#include <iostream>

#include <algorithm>
#include <queue>
#include <vector>

#include "realm/Realm.h"
#include "util/AStar.h"

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

	static inline size_t heuristic(const Position &a, const Position &b) {
		return std::abs(a.row - b.row) + std::abs(a.column - b.column);
	}

	static inline void getNeighbors(const std::shared_ptr<Realm> &realm, const Position &position, std::vector<Position> &next) {
		next.clear();

		if (0 < position.row) {
			const Position next_pos(position.row - 1, position.column);
			if (realm->pathMap[realm->getIndex(next_pos)] != 0)
				next.emplace_back(next_pos);
		}

		if (0 < position.column) {
			const Position next_pos(position.row, position.column - 1);
			if (realm->pathMap[realm->getIndex(next_pos)] != 0)
				next.emplace_back(next_pos);
		}

		if (position.row < realm->getHeight() - 1) {
			const Position next_pos(position.row + 1, position.column);
			if (realm->pathMap[realm->getIndex(next_pos)] != 0)
				next.emplace_back(next_pos);
		}

		if (position.column < realm->getWidth() - 1) {
			const Position next_pos(position.row, position.column + 1);
			if (realm->pathMap[realm->getIndex(next_pos)] != 0)
				next.emplace_back(next_pos);
		}
	}

	// Credit: https://www.redblobgames.com/pathfinding/a-star/implementation.html#cplusplus
	bool simpleAStar(const std::shared_ptr<Realm> &realm, const Position &start, const Position &goal, std::vector<Position> &path) {
		std::unordered_map<Position, Position> moves;
		std::unordered_map<Position, size_t> costs;
		PriorityQueue<Position, size_t> frontier;
		frontier.put(start, 0);

		moves[start] = start;
		costs[start] = 0;

		std::vector<Position> next_positions;
		next_positions.reserve(4);

		while (!frontier.empty()) {
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