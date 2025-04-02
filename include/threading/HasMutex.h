#pragma once

#include "threading/Lockable.h"

#include <mutex>
#include <shared_mutex>

namespace Game3 {
	/** For protecting subclass fields that aren't instances of Lockable. */
	template <typename T, typename M = DefaultMutex>
	class HasMutex {
		protected:
			mutable M internalMutex;

			HasMutex() = default;

		public:
			virtual ~HasMutex() = default;
			virtual std::shared_lock<M> sharedLock() const { return std::shared_lock(internalMutex); }
			virtual std::unique_lock<M> uniqueLock() const { return std::unique_lock(internalMutex); }

			template <typename Fn>
			requires std::invocable<Fn>
			decltype(auto) withShared(Fn &&function) const {
				auto lock = sharedLock();
				return function();
			}

			template <typename Fn>
			requires std::invocable<Fn, const T &>
			decltype(auto) withShared(Fn &&function) const {
				auto lock = sharedLock();
				return function(static_cast<const T &>(*this));
			}

			template <typename Fn>
			requires (std::invocable<Fn, T &> && !std::invocable<Fn, const T &>)
			decltype(auto) withShared(Fn &&function) {
				auto lock = sharedLock();
				return function(static_cast<T &>(*this));
			}

			template <typename Fn>
			requires std::invocable<Fn>
			decltype(auto) withUnique(Fn &&function) const {
				auto lock = uniqueLock();
				return function();
			}

			template <typename Fn>
			requires std::invocable<Fn, const T &>
			decltype(auto) withUnique(Fn &&function) const {
				auto lock = uniqueLock();
				return function(static_cast<const T &>(*this));
			}

			template <typename Fn>
			requires (std::invocable<Fn, T &> && !std::invocable<Fn, const T &>)
			decltype(auto) withUnique(Fn &&function) {
				auto lock = uniqueLock();
				return function(static_cast<T &>(*this));
			}
	};
}
