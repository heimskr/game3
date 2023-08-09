#pragma once

#include <mutex>
#include <shared_mutex>

namespace Game3 {
	/** For protecting subclass fields that aren't instances of Lockable. */
	template <typename T = std::shared_mutex>
	class HasMutex {
		protected:
			mutable T internalMutex;

			HasMutex() = default;

		public:
			inline auto sharedLock() const { return std::shared_lock(internalMutex); }
			inline auto uniqueLock() const { return std::unique_lock(internalMutex); }
	};
}
