#pragma once

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <ostream>
#include <string>

#include "types/Types.h"

namespace Game3 {
	struct QuadtreeBox {
		public:
			Index left {};
			Index top {};
			Index width {};
			Index height {};

			/** {TL, TR, BL, BR} */
			std::array<std::unique_ptr<QuadtreeBox>, 4> children {};
			inline bool isLeaf() const { return width <= 1 && height <= 1; }
			inline bool inBounds(Index row, Index column) const { return top <= row && row < top + height && left <= column && column < left + height; }
			inline bool full() const { return isLeaf() || (full(0) && full(1) && full(2) && full(3)); }
			inline bool is(Index row, Index column) { return width == 1 && height == 1 && top == row && left == column; }
			bool contains(Index row, Index column) const;

			/** Returns false if this is a leaf or if the given coordinate isn't in this box's bounds, or if the position is already present. */
			bool add(Index row, Index column);
			bool remove(Index row, Index column);
			void reset();

			inline std::optional<QuadtreeBox> topLeft()     const { if (isLeaf()) return std::nullopt; return QuadtreeBox{left,             top,              half(width), half(height)}; }
			inline std::optional<QuadtreeBox> topRight()    const { if (isLeaf()) return std::nullopt; return QuadtreeBox{left + width / 2, top,              width / 2,   half(height)}; }
			inline std::optional<QuadtreeBox> bottomLeft()  const { if (isLeaf()) return std::nullopt; return QuadtreeBox{left,             top + height / 2, half(width), height / 2  }; }
			inline std::optional<QuadtreeBox> bottomRight() const { if (isLeaf()) return std::nullopt; return QuadtreeBox{left + width / 2, top + height / 2, width / 2,   height / 2  }; }

			operator std::string() const;

		private:
			static inline Index half(Index value) { return 1 < value? value / 2 : 1; }
			inline bool full(size_t index) const { return children[index] && children[index]->full(); }
			bool remove(std::unique_ptr<QuadtreeBox> &, Index row, Index column);

		friend class Quadtree;
	};

	class Quadtree {
		private:
			QuadtreeBox root;
			std::function<bool(Index, Index)> predicate;

		public:
			/** If this returns true, iteration will end. */
			using Visitor = std::function<bool(const QuadtreeBox &)>;

			Quadtree() = delete;
			Quadtree(Index width, Index height);
			Quadtree(Index width, Index height, decltype(predicate));

			inline bool add(Index row, Index column) { return root.add(row, column); }
			inline bool remove(Index row, Index column) { return root.remove(row, column); }
			inline void reset() { root.reset(); }

			inline const QuadtreeBox & getRoot() const { return root; }
			bool contains(Index row, Index column) const;
			/** Iterates over each full box and returns true if iteration was canceled. */
			bool iterateFull(const Visitor &) const;
			void absorb(bool do_reset = true);
	};
}
