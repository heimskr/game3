#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>

namespace Game3 {
	class RWLock {
		public:
			std::shared_lock<std::shared_timed_mutex> lockRead();
			/** If the writer has been waiting for at least `patience`, new read lock attempts will block until after the writer has acquired and released the lock. */
			std::unique_lock<std::shared_timed_mutex> lockWrite(std::chrono::milliseconds patience);

		private:
			std::atomic_bool blockReaders {false};
			std::shared_timed_mutex timedMutex;
			std::mutex conditionMutex;
			std::condition_variable conditionVariable;
			std::mutex writerMutex;
	};
}
