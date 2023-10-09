#pragma once

#include <chrono>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>

namespace Game3 {
	class Timer {
		public:
			static std::map<std::string, std::chrono::nanoseconds> times;
			static std::map<std::string, size_t> counts;
			static std::shared_mutex mutex;

			std::chrono::system_clock::time_point start;
			const std::string name;

			Timer(const std::string &name_);
			~Timer();

			std::chrono::nanoseconds difference() const;
			void stop();
			void restart();

			static void summary(double threshold = 0.0);
			static void clear();

			static auto sharedLock() { return std::shared_lock(mutex); }
			static auto uniqueLock() { return std::unique_lock(mutex); }

		private:
			bool stopped = false;
	};
}
