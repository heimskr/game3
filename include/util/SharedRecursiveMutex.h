#pragma once

#include <atomic>
#include <mutex>
#include <thread>

namespace Game3 {
	// https://stackoverflow.com/a/36624355
	class SharedRecursiveMutex: private std::shared_mutex {
		public:
			void lock() {
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

			void lock_shared() {
				if (owner != std::this_thread::get_id())
					std::shared_mutex::lock_shared();
			}

			void unlock_shared() {
				if (owner != std::this_thread::get_id())
					std::shared_mutex::unlock_shared();
			}

			void unlock() {
				if (1 < count) {
					// recursive unlocking
					--count;
				} else {
					// normal unlocking
					owner = std::thread::id();
					count = 0;
					std::shared_mutex::unlock();
				}
			}

		private:
			std::atomic<std::thread::id> owner;
			int count;
	};
}
