#pragma once

#include <memory>
#include <set>

namespace Game3 {
	template <typename T>
	struct WeakCompare {
		bool operator()(std::weak_ptr<T> left, std::weak_ptr<T> right) const {
			auto llock = left.lock();
			auto rlock = right.lock();
			if (!rlock)
				return false;
			if (!llock)
				return true;
			return llock.get() < rlock.get();
		}
	};

	template <typename T>
	using WeakSet = std::set<std::weak_ptr<T>, WeakCompare<T>>;
}
