#pragma once

#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <thread>

namespace Game3 {
	// https://stackoverflow.com/a/36624355
	class SharedRecursiveMutex: private std::shared_mutex, private std::recursive_mutex {
		public:
			void lock();
			void lock_shared();
			void unlock_shared();
			void unlock();
			bool try_lock_shared();
			bool try_lock();

		private:
			std::atomic<std::thread::id> owner;
			std::atomic_int count;
	};
}
