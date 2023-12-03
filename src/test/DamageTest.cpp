#include "algorithm/DamageCalculation.h"
#include "threading/ThreadContext.h"
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

		for (int i = 0; i < 1'000'000; ++i) {
			const auto quantile = boost::math::quantile(distribution, uniform(rng));
			++samples[int(factor * quantile)];
		}

		printHistogram(samples);
	}

	void damageTest(HitPoints weapon_damage, int defense, int variability, double attacker_luck, double defender_luck) {
		std::map<HitPoints, size_t> histogram;

		for (int i = 0; i < 1'000'000; ++i) {
			std::uniform_int_distribution<int> defense_distribution(0, 99);
			HitPoints damage = calculateDamage(weapon_damage, variability, attacker_luck);

			for (int i = 0; i < defense; ++i) {
				if (damage == 0)
					break;

				if (defense_distribution(threadContext.rng) < 10 * defender_luck)
					--damage;
			}

			++histogram[damage];
		}

		printHistogram(histogram, 0.5);
		std::cout << '\n';
	}
}
