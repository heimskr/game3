#include "entity/Entity.h"
#include "entity/EntityZCompare.h"

namespace Game3 {
	bool EntityZCompare::operator()(const std::shared_ptr<Entity> &left, const std::shared_ptr<Entity> &right) const {
		assert(left);
		assert(right);
		const int z_left  = left->getZIndex();
		const int z_right = right->getZIndex();
		if (z_left < z_right)
			return true;
		if (z_left > z_right)
			return false;
		return left < right;
	}

	bool EntityZCompare::operator()(const std::weak_ptr<Entity> &left, const std::weak_ptr<Entity> &right) const {
		auto left_locked = left.lock();
		auto right_locked = right.lock();

		if (!left_locked || !right_locked)
			return bool(left_locked);

		return (*this)(left_locked, right_locked);
	}
}
