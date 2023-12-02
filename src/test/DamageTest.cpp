#include "algorithm/DamageCalculation.h"
#include "types/Types.h"
#include "util/Histogram.h"

#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <sys/ioctl.h>
#include <unistd.h>

#include <boost/math/distributions/skew_normal.hpp>

namespace Game3 {
	void skewTest(double location, double scale, double shape) {
		boost::math::skew_normal_distribution<double> distribution(location, scale, shape);

		std::random_device dev;
		std::default_random_engine rng(dev());
		std::uniform_real_distribution<double> uniform(0., 1.);

		std::map<int, size_t> samples;

		struct winsize w;
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

		const int factor = w.ws_col;

		for (int i = 0; i < 10'000'000; ++i) {
			const auto quantile = boost::math::quantile(distribution, uniform(rng));
			++samples[int(factor * quantile)];
		}

		size_t max_samples = 0;
		for (const auto [n, count]: samples)
			max_samples = std::max(max_samples, count);

		const int digits = std::floor(std::log10(factor)) + 1;
		const int height = w.ws_row - digits - 2;

		const double line_scale = double(height) / max_samples;

		auto move = [&](int x, int y) {
			std::cout << "\e[" << (y + 1) << ';' << (x + 1) << 'H';
		};

		std::cout << "\e[2J\e[H";

		for (int i = 0; i < factor; ++i) {
			const size_t max = std::ceil(samples[i] * line_scale);

			if (i == factor / 2)
				std::cout << "\e[31m";

			for (size_t j = 0; j < max; ++j) {
				move(i, height - max + j);
				std::cout << "█";
			}

			int digit = 0;
			for (const char ch: std::to_string(i)) {
				move(i, height + digit++);
				std::cout << ch;
			}

			if (i == factor / 2)
				std::cout << "\e[39m";
		}
	}

	void damageTest(HitPoints weapon_damage, int defense, int variability, double attacker_luck, double defender_luck) {
		std::map<HitPoints, size_t> histogram;

		for (int i = 0; i < 1'000'000; ++i)
			++histogram[calculateDamage(weapon_damage, defense, variability, attacker_luck, defender_luck)];

		printHistogram(histogram, 0.5);
		std::cout << '\n';
	}
}
