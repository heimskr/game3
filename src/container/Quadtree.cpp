#include "Tilemap.h"
#include "container/Quadtree.h"

namespace Game3 {
	bool Box::add(Index row, Index column) {
		if (isLeaf() || !inBounds(row, column))
			return false;

		const bool in_top  = row    < top  + height / 2;
		const bool in_left = column < left + width  / 2;

		if (in_top) {
			if (in_left) {
				auto &child = children[0];
				const bool was_false = child != nullptr;
				if (!child)
					child = std::make_unique<Box>(*topLeft());
				return child->add(row, column) || was_false;
			}
			auto &child = children[1];
			const bool was_false = child != nullptr;
			if (!child)
				child = std::make_unique<Box>(*topRight());
			return child->add(row, column) || was_false;
		}

		if (in_left) {
			auto &child = children[2];
			const bool was_false = child != nullptr;
			if (!child)
				child = std::make_unique<Box>(*bottomLeft());
			return child->add(row, column) || was_false;
		}

		auto &child = children[3];
		const bool was_false = child != nullptr;
		if (!child)
			child = std::make_unique<Box>(*bottomRight());
		return child->add(row, column) || was_false;
	}

	bool Box::contains(Index row, Index column) const {
		if (isLeaf() && top == row && left == column)
			return true;

		const bool in_top  = row    < top  + height / 2;
		const bool in_left = column < left + width  / 2;

		if (in_top) {
			if (in_left)
				return children[0] && children[0]->contains(row, column);
			return children[1] && children[1]->contains(row, column);
		}

		if (in_left)
			return children[2] && children[2]->contains(row, column);
		return children[3] && children[3]->contains(row, column);
	}

	Quadtree::Quadtree(Index width, Index height):
		root{0, 0, width, height} {}

	Quadtree::Quadtree(const Tilemap &tilemap):
		Quadtree(tilemap.width, tilemap.height) {}

	void Quadtree::add(Index row, Index column) {
		root.add(row, column);
	}
}
