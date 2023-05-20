#include "threading/ThreadPool.h"

namespace Game3 {
	ThreadPool::ThreadPool(size_t size_): size(size_) {
		pool.reserve(size);
	}

	ThreadPool::~ThreadPool() {
		join();
	}

	void ThreadPool::start() {
		if (joining || active.exchange(true))
			return;
		assert(pool.empty());
		for (size_t thread_index = 0; thread_index < size; ++thread_index)
			pool.emplace_back([this, thread_index] {
				threadContext = {};
				while (active) {
					std::unique_lock lock(workMutex);
					workCV.wait(lock);
					if (auto job = workQueue.tryTake()) {
						assert(*job);
						(*job)(*this, thread_index);
					}
				}
			});
	}

	void ThreadPool::join() {
		if (active.exchange(false) && !joining.exchange(true)) {
			workQueue.clear();
			workCV.notify_all();
			for (auto &thread: pool)
				thread.join();
			pool.clear();
			joining = false;
		}
	}

	bool ThreadPool::add(const Function &function) {
		if (active) {
			// TODO: race condition if active goes false during this block
			workQueue.push(function);
			workCV.notify_one();
			return true;
		}

		return false;
	}
}
