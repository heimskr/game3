#pragma once

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

#include "Log.h"
#include "threading/ThreadContext.h"
#include "threading/MTQueue.h"

namespace Game3 {
	class ThreadPool {
		public:
			using Function = std::function<void(ThreadPool &, size_t)>;

			ThreadPool(size_t size_);

			~ThreadPool();

			inline bool isActive() const { return active; }
			inline auto getSize() const { return size; }

			void start();
			void join();
			bool add(const Function &);

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
