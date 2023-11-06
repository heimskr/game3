#pragma once

#include "threading/Lockable.h"

#include <mutex>
#include <shared_mutex>

namespace Game3 {
	/** For protecting subclass fields that aren't instances of Lockable. */
	template <typename M = DefaultMutex>
	class HasMutex {
		protected:
			mutable M internalMutex;

			HasMutex() = default;

		public:
			virtual ~HasMutex() = default;
			virtual std::shared_lock<M> sharedLock() const { return std::shared_lock(internalMutex); }
			virtual std::unique_lock<M> uniqueLock() const { return std::unique_lock(internalMutex); }
	};
}
