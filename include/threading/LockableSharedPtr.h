#pragma once

#include "threading/Lockable.h"

#include <memory>

namespace Game3 {
	template <typename T, typename M = DefaultMutex>
	struct LockableSharedPtr: Lockable<std::shared_ptr<T>, M> {
		using Base = std::shared_ptr<T>;

		using Lockable<std::shared_ptr<T>, M>::Lockable;

		operator std::shared_ptr<T>() const noexcept {
			auto lock = this->sharedLock();
			return static_cast<std::shared_ptr<T>>(*this);
		}

		void swap(std::shared_ptr<T> &other) noexcept {
			auto unique_lock = this->uniqueLock();
			Base::swap(other);
		}

		template <typename M2>
		void swap(LockableSharedPtr<T, M2> &other) noexcept {
			auto this_unique_lock  = this->uniqueLock();
			auto other_unique_lock = other.uniqueLock();
			Base::swap(other.getBase());
		}

		void reset() noexcept {
			auto unique_lock = this->uniqueLock();
			Base::reset();
		}

		template <class U>
		void reset(U *pointer) {
			auto unique_lock = this->uniqueLock();
			Base::reset(pointer);
		}

		template <class U, class D>
		void reset(U *pointer, D deleter) {
			auto unique_lock = this->uniqueLock();
			Base::reset(pointer, deleter);
		}

		template <class U, class D, class Alloc>
		void reset(U *pointer, D deleter, Alloc allocator) {
			auto unique_lock = this->uniqueLock();
			Base::reset(pointer, deleter, allocator);
		}

		T * get() const noexcept {
			auto shared_lock = this->sharedLock();
			return Base::get();
		}

		T & operator*() const noexcept {
			auto shared_lock = this->sharedLock();
			return Base::operator*();
		}

		T * operator->() const noexcept {
			auto shared_lock = this->sharedLock();
			return Base::operator->();
		}

		auto use_count() const noexcept {
			auto shared_lock = this->sharedLock();
			return Base::use_count();
		}

		bool unique() const noexcept {
			auto shared_lock = this->sharedLock();
			return Base::unique();
		}

		explicit operator bool() const noexcept {
			auto shared_lock = this->sharedLock();
			return Base::operator bool();
		}
	};
}
