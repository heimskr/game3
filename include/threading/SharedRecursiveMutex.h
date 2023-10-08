#pragma once

#include <atomic>
#include <mutex>
#include <shared_mutex>
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
				// This is not ideal!
				if (owner != std::this_thread::get_id())
					std::shared_mutex::lock_shared();
			}

			void unlock_shared() {
				// This is not ideal either.
				if (owner != std::this_thread::get_id())
					std::shared_mutex::unlock_shared();
			}

			void unlock() {
				if (count.fetch_sub(1) == 1) {
					// normal unlocking
					owner = std::thread::id();
					count = 0;
					std::shared_mutex::unlock();
				}
			}

			bool try_lock_shared() {
				if (owner != std::this_thread::get_id())
					return std::shared_mutex::try_lock_shared();

				return true;
			}

			bool try_lock() {
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

		private:
			std::atomic<std::thread::id> owner;
			std::atomic_int count;
	};
}
