#pragma once

#include <mutex>
#include <shared_mutex>

namespace Game3 {
	/** For protecting subclass fields that aren't instances of Lockable. */
	template <typename T = std::shared_mutex>
	class HasMutex {
		protected:
			T internalMutex;

			HasMutex() = default;

		public:
			inline auto sharedLock() { return std::shared_lock(internalMutex); }
			inline auto uniqueLock() { return std::unique_lock(internalMutex); }
	};
}
