#pragma once

#include <chrono>
#include <map>
#include <string>
#include <vector>

namespace Game3 {
	class Timer {
		public:
			static std::map<std::string, std::chrono::nanoseconds> times;
			static std::map<std::string, size_t> counts;

			std::chrono::system_clock::time_point start;
			const std::string name;

			Timer(const std::string &name_);
			~Timer();

			std::chrono::nanoseconds difference() const;
			void stop();

			static void summary(double threshold = 0.0);
			static void clear();

		private:
			bool stopped = false;
	};
}
