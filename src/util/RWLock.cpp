#include "util/RWLock.h"

namespace Game3 {
	std::shared_lock<std::shared_timed_mutex> RWLock::lockRead() {
		if (blockReaders) {
			std::unique_lock condition_lock(conditionMutex);
			conditionVariable.wait(condition_lock, [this] { return !blockReaders; });
		}

		return std::shared_lock(timedMutex);
	}

	std::unique_lock<std::shared_timed_mutex> RWLock::lockWrite(std::chrono::milliseconds patience) {
		std::unique_lock writer_lock(writerMutex);
		std::unique_lock attempted_lock(timedMutex, patience);
		if (attempted_lock)
			return attempted_lock;
		blockReaders = true;
		attempted_lock = std::unique_lock(timedMutex);
		blockReaders = false;
		conditionVariable.notify_all();
		return attempted_lock;
	}
}
