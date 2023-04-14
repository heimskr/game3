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

	bool Box::remove(Index row, Index column) {
		// isLeaf() shouldn't return true here unless the quadtree is 1x1, so we can return false instead of throwing an exception. */
		if (isLeaf() || !inBounds(row, column))
			return false;

		const bool in_top  = row    < top  + height / 2;
		const bool in_left = column < left + width  / 2;

		if (in_top) {
			if (in_left)
				return remove(children[0], row, column);
			return remove(children[1], row, column);
		}

		if (in_left)
			return remove(children[2], row, column);
		return remove(children[3], row, column);
	}

	bool Box::remove(std::unique_ptr<Box> &child, Index row, Index column) {
		if (child) {
			if (child->is(row, column)) {
				child.reset();
				return true;
			}

			return child->remove(row, column);
		}

		return false;
	}

	void Box::reset() {
		children[0].reset();
		children[1].reset();
		children[2].reset();
		children[3].reset();
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

	Quadtree::Quadtree(std::shared_ptr<Tilemap> tilemap_, decltype(predicate) predicate_):
	tilemap(std::move(tilemap_)),
	root{tilemap->width, tilemap->height},
	predicate(std::move(predicate_)) {
		absorb();
	}

	bool Quadtree::contains(Index row, Index column) const {
		if (tilemap && predicate)
			return predicate(*tilemap, row, column);
		return root.contains(row, column);
	}

	void Quadtree::absorb() {
		const Tilemap &map = *tilemap;
		for (size_t row = 0; row < root.height; ++row)
			for (size_t column = 0; column < root.width; ++column)
				if (predicate(map, row, column))
					add(row, column);
	}
}
