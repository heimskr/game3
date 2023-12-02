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

		for (int i = 0; i < 1'000'000; ++i) {
			const auto quantile = boost::math::quantile(distribution, uniform(rng));
			++samples[int(factor * quantile)];
		}

		printHistogram(samples);
	}

	void damageTest(HitPoints weapon_damage, int defense, int variability, double attacker_luck, double defender_luck) {
		std::map<HitPoints, size_t> histogram;

		for (int i = 0; i < 1'000'000; ++i)
			++histogram[calculateDamage(weapon_damage, defense, variability, attacker_luck, defender_luck)];

		printHistogram(histogram, 0.5);
		std::cout << '\n';
	}
}
