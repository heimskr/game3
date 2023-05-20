#pragma once

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

#include "threading/ThreadContext.h"
#include "threading/MTQueue.h"

namespace Game3 {
	class ThreadPool {
		public:
			using Function = std::function<void(ThreadPool &, size_t)>;

			ThreadPool(size_t size_): size(size_) {
				pool.reserve(size);
			}

			~ThreadPool() {
				join();
			}

			inline bool isActive() const { return active; }
			inline auto getSize() const { return size; }

			void start() {
				if (joining || active.exchange(true))
					return;
				assert(pool.empty());
				for (size_t thread_index = 0; thread_index < size; ++thread_index)
					pool.emplace_back([this, thread_index] {
						threadContext = {};
						while (active) {
							std::unique_lock lock(workMutex);
							workCV.wait(lock);
							if (auto job = workQueue.tryTake())
								(*job)(*this, thread_index);
						}
					});
			}

			void join() {
				if (active.exchange(false) && !joining.exchange(true)) {
					workQueue.clear();
					workCV.notify_all();
					for (auto &thread: pool)
						thread.join();
					pool.clear();
					joining = false;
				}
			}

			inline bool add(const Function &function) {
				if (active) {
					// TODO: race condition if active goes false during this block
					workQueue.push(function);
					workCV.notify_one();
					return true;
				}

				return false;
			}

		protected:
			const size_t size;
			MTQueue<Function> workQueue;
			std::condition_variable workCV;
			std::mutex workMutex;
			std::vector<std::thread> pool;
			std::atomic_bool active = false;
			std::atomic_bool joining = false;
	};
}
