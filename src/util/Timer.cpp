#include <algorithm>
#include <iostream>

#include "util/Timer.h"

namespace Game3 {
	std::map<std::string, std::chrono::nanoseconds> Timer::times;
	std::map<std::string, size_t> Timer::counts;
	std::shared_mutex Timer::mutex;
	std::atomic_bool Timer::globalEnabled{true};

	Timer::Timer(const std::string &name_):
		start(std::chrono::system_clock::now()), name(name_) {}

	Timer::~Timer() {
		stop();
	}

	std::chrono::nanoseconds Timer::difference() const {
		return std::chrono::system_clock::now() - start;
	}

	void Timer::stop() {
		if (stopped)
			return;
		auto lock = uniqueLock();
		times[name] += difference();
		++counts[name];
		stopped = true;
	}

	void Timer::restart() {
		start = std::chrono::system_clock::now();
		stopped = false;
	}

	void Timer::summary(double threshold) {
		if (!globalEnabled)
			return;

		auto lock = sharedLock();

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
				std::cerr << std::format("    \e[1m{}{}\e[22m took \e[32m{}\e[39m seconds", *name, std::string(max_length - name->size(), ' '), nanos / 1e9);
				const size_t count = counts.at(*name);
				if (1 < count)
					std::cerr << std::format(" (average: \e[33m{}\e[39m over \e[1m{}\e[22m instances)", nanos / count / 1e9, count);
				std::cerr << '\n';
			}
		}
	}

	void Timer::clear() {
		auto lock = uniqueLock();
		times.clear();
		counts.clear();
	}
}
