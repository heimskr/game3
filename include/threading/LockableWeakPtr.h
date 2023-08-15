#pragma once

#include "threading/Lockable.h"

#include <memory>

namespace Game3 {
	template <typename T, typename M = std::shared_mutex>
	struct LockableWeakPtr: Lockable<std::weak_ptr<T>, M> {
		using Base = std::weak_ptr<T>;

		using Lockable<std::weak_ptr<T>, M>::Lockable;

		auto lock() const noexcept {
			auto shared_lock = this->sharedLock();
			return Base::lock();
		}

		void swap(std::weak_ptr<T> &other) noexcept {
			auto unique_lock = this->uniqueLock();
			Base::swap(other);
		}

		template <typename M2>
		void swap(LockableWeakPtr<T, M2> &other) noexcept {
			auto this_unique_lock  = this->uniqueLock();
			auto other_unique_lock = other.uniqueLock();
			Base::swap(other.getBase());
		}

		void reset() noexcept {
			auto unique_lock = this->uniqueLock();
			Base::reset();
		}

		auto use_count() const noexcept {
			auto shared_lock = this->sharedLock();
			return Base::use_count();
		}

		auto expired() const noexcept {
			auto shared_lock = this->sharedLock();
			return Base::expired();
		}
	};
}
