#pragma once

#include "util/Concepts.h"

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace Game3 {
	/** Useful for when you want to wait for a certain number of jobs in a thread pool to finish. */
	class Waiter {
		public:
			Waiter(size_t remaining_) noexcept;

			Waiter & operator--() noexcept;
			void wait();
			bool isDone() const noexcept;
			void reset(size_t, bool force = false);

			std::cv_status waitFor(Duration auto duration) {
				std::unique_lock lock(mutex);
				if (remaining > 0) {
					return condition.wait_for(lock, duration, [this] { return remaining == 0; })? std::cv_status::no_timeout : std::cv_status::timeout;
				}
				return std::cv_status::no_timeout;
			}

			std::cv_status waitUntil(TimePoint auto time_point) {
				std::unique_lock lock(mutex);
				if (remaining > 0) {
					return condition.wait_until(lock, time_point, [this] { return remaining == 0; })? std::cv_status::no_timeout : std::cv_status::timeout;
				}
				return std::cv_status::no_timeout;
			}

		private:
			std::condition_variable condition;
			std::atomic_size_t remaining;
			std::mutex mutex;
	};
}
