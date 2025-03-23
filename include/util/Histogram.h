#pragma once

#ifndef __MINGW32__

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>

namespace Game3 {
	template <typename Map>
	void printHistogram(const Map &samples, double scale = 1.0) {
		struct winsize w;
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

		const int factor = samples.size();

		size_t max_samples = 0;
		size_t max_n = 0;
		for (const auto &[n, count]: samples) {
			max_samples = std::max(max_samples, size_t(count));
			max_n = std::max(max_n, size_t(n));
		}

		const int digits = 0 < factor? std::floor(std::log10(max_n)) + 1 : 0;
		const int height = w.ws_row - digits - 2;

		const double line_scale = scale * (max_samples != 0? double(height) / max_samples : 0);

		auto move = [&](int x, int y) {
			std::cout << "\e[" << (y + 1) << ';' << (x + 1) << 'H';
		};

		std::cout << "\e[2J\e[H";

		size_t i = 0;

		if ((digits + 1) * samples.size() <= w.ws_col) {
			for (const auto &[n, count]: samples) {
				const size_t max = std::ceil(count * line_scale);

				if (i == samples.size() / 2)
					std::cout << "\e[31m";

				for (size_t j = 0; j < max; ++j) {
					move(i * (digits + 1), height - max + j);
					for (int k = 0; k <= digits; ++k)
						std::cout << "█";
				}

				move(i * (digits + 1), height);
				std::cout << n;

				if (i == samples.size() / 2)
					std::cout << "\e[39m";

				++i;
			}
		} else {
			for (const auto &[n, count]: samples) {
				const size_t max = std::ceil(count * line_scale);

				if (i == samples.size() / 2)
					std::cout << "\e[31m";

				for (size_t j = 0; j < max; ++j) {
					move(i, height - max + j);
					std::cout << "█";
				}

				int digit = 0;
				for (const char ch: std::to_string(n)) {
					move(i, height + digit++);
					std::cout << ch;
				}

				if (i == samples.size() / 2)
					std::cout << "\e[39m";

				++i;
			}
		}
	}
}

#else

namespace Game3 {
	template <typename Map>
	void printHistogram(const Map &, double = 1.0) {}
}

#endif
