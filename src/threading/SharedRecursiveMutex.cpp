#include "threading/SharedRecursiveMutex.h"

namespace Game3 {
#ifdef __MINGW32__
	// shared_mutex is exploding for seemingly no reason
	constexpr static bool uniqueOnly = true;
#else
	constexpr static bool uniqueOnly = false;
#endif

	void SharedRecursiveMutex::lock() {
		if constexpr (uniqueOnly) {
			std::recursive_mutex::lock();
			return;
		}

		auto this_id = std::this_thread::get_id();
		if (owner == this_id) {
			// recursive locking
			++count;
		} else {
			// normal locking
			std::shared_mutex::lock();
			owner = this_id;
			count = 1;
		}
	}

	void SharedRecursiveMutex::lock_shared() {
		if constexpr (uniqueOnly) {
			std::recursive_mutex::lock();
			return;
		}

		// This is not ideal!
		if (owner != std::this_thread::get_id()) {
			std::shared_mutex::lock_shared();
		}
	}

	void SharedRecursiveMutex::unlock_shared() {
		if constexpr (uniqueOnly) {
			std::recursive_mutex::unlock();
			return;
		}

		// This is not ideal either.
		if (owner != std::this_thread::get_id()) {
			std::shared_mutex::unlock_shared();
		}
	}

	void SharedRecursiveMutex::unlock() {
		if constexpr (uniqueOnly) {
			std::recursive_mutex::unlock();
			return;
		}

		if (count.fetch_sub(1) == 1) {
			// normal unlocking
			owner = std::thread::id();
			count = 0;
			std::shared_mutex::unlock();
		}
	}

	bool SharedRecursiveMutex::try_lock_shared() {
		if constexpr (uniqueOnly) {
			return std::recursive_mutex::try_lock();
		}

		if (owner != std::this_thread::get_id()) {
			return std::shared_mutex::try_lock_shared();
		}

		return true;
	}

	bool SharedRecursiveMutex::try_lock() {
		if constexpr (uniqueOnly) {
			return std::recursive_mutex::try_lock();
		}

		auto this_id = std::this_thread::get_id();
		if (owner == this_id) {
			if (std::shared_mutex::try_lock()) {
				++count;
				return true;
			}

			return false;
		}

		if (std::shared_mutex::try_lock()) {
			owner = this_id;
			count = 1;
			return true;
		}

		return false;
	}
}
