#include <algorithm>
#include <iostream>

#include "util/Timer.h"

namespace Game3 {
	std::map<std::string, std::chrono::nanoseconds> Timer::times;
	std::map<std::string, size_t> Timer::counts;

	Timer::Timer(const std::string &name_):
		start(std::chrono::system_clock::now()), name(name_) {}

	Timer::~Timer() {
		stop();
	}

	std::chrono::nanoseconds Timer::difference() const {
		return std::chrono::system_clock::now() - start;
	}

	void Timer::stop() {
		if (!stopped) {
			times[name] += difference();
			++counts[name];
			stopped = true;
		}
	}

	void Timer::summary(double threshold) {
		if (!times.empty()) {
			std::cerr << "Timer summary:\n";

			std::vector<const std::string *> names;
			names.reserve(times.size());
			size_t max_length = 0;

			for (const auto &[name, nanos]: times) {
				if (nanos.count() / 1e9 < threshold)
					continue;
				names.push_back(&name);
				max_length = std::max(name.size(), max_length);
			}

			std::sort(names.begin(), names.end(), [](const std::string *left, const std::string *right) {
				return times.at(*left) > times.at(*right);
			});

			for (const std::string *name: names) {
				const double nanos = times.at(*name).count();
				std::cerr << "    \e[1m" << *name << std::string(max_length - name->size(), ' ') << "\e[22m took \e[32m"
				          << (nanos / 1e9) << "\e[39m seconds";
				const size_t count = counts.at(*name);
				if (1 < count)
					std::cerr << " (average: \e[33m" << (nanos / double(count) / 1e9) << "\e[39m over \e[1m" << count
					          << "\e[22m instances)";
				std::cerr << '\n';
			}
		}
	}

	void Timer::clear() {
		times.clear();
		counts.clear();
	}
}
