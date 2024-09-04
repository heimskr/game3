#include "container/Quadtree.h"

namespace Game3 {
	bool QuadtreeBox::add(Index row, Index column) {
		if (isLeaf() || !inBounds(row, column))
			return false;

		const bool in_top  = row    < top  + height / 2;
		const bool in_left = column < left + width  / 2;

		if (in_top) {
			if (in_left) {
				auto &child = children[0];
				const bool was_false = child != nullptr;
				if (!child)
					child = std::make_unique<QuadtreeBox>(*topLeft());
				return child->add(row, column) || was_false;
			}
			auto &child = children[1];
			const bool was_false = child != nullptr;
			if (!child)
				child = std::make_unique<QuadtreeBox>(*topRight());
			return child->add(row, column) || was_false;
		}

		if (in_left) {
			auto &child = children[2];
			const bool was_false = child != nullptr;
			if (!child)
				child = std::make_unique<QuadtreeBox>(*bottomLeft());
			return child->add(row, column) || was_false;
		}

		auto &child = children[3];
		const bool was_false = child != nullptr;
		if (!child)
			child = std::make_unique<QuadtreeBox>(*bottomRight());
		return child->add(row, column) || was_false;
	}

	bool QuadtreeBox::remove(Index row, Index column) {
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

	bool QuadtreeBox::remove(std::unique_ptr<QuadtreeBox> &child, Index row, Index column) {
		if (child) {
			if (child->is(row, column)) {
				child.reset();
				return true;
			}

			return child->remove(row, column);
		}

		return false;
	}

	void QuadtreeBox::reset() {
		children[0].reset();
		children[1].reset();
		children[2].reset();
		children[3].reset();
	}

	bool QuadtreeBox::contains(Index row, Index column) const {
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

	QuadtreeBox::operator std::string() const {
		return "QuadtreeBox[(" + std::to_string(top) + ", " + std::to_string(left) + "), " + std::to_string(height) + "h x " + std::to_string(width) + "w]";
	}

	Quadtree::Quadtree(Index width, Index height):
		root{0, 0, width, height} {}

	Quadtree::Quadtree(Index width, Index height, decltype(predicate) predicate_):
		root{0, 0, width, height},
		predicate(std::move(predicate_)) {
			absorb();
		}

	bool Quadtree::contains(Index row, Index column) const {
		if (predicate)
			return predicate(row, column);
		return root.contains(row, column);
	}

	void Quadtree::absorb(bool do_reset) {
		if (do_reset)
			reset();

		for (Index row = 0; row < root.height; ++row)
			for (Index column = 0; column < root.width; ++column)
				if (predicate(row, column))
					add(row, column);
	}

	bool Quadtree::iterateFull(const Visitor &visitor) const {
		std::vector<const QuadtreeBox *> boxes {&root};
		while (!boxes.empty()) {
			const QuadtreeBox *box = boxes.back();
			boxes.pop_back();

			if (box->full()) {
				if (visitor(*box))
					return true;
			} else {
				for (size_t i = 0; i < 4; ++i) {
					if (auto child = box->children[i].get())
						boxes.push_back(child);
				}
			}
		}

		return false;
	}
}
