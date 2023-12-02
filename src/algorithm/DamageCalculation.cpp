#include "algorithm/DamageCalculation.h"
#include "threading/ThreadContext.h"

#include <boost/math/distributions/skew_normal.hpp>

namespace Game3 {
	HitPoints calculateDamage(HitPoints weapon_damage, int defense, int variability, double attacker_luck, double defender_luck) {
		boost::math::skew_normal_distribution<double> skew_distribution(0, 0.15, attacker_luck); // With luck = 0, this ranges from roughly -0.25 to 0.25
		std::uniform_real_distribution<double> uniform(0., 1.);
		std::uniform_int_distribution<int> defense_distribution(0, 99);
		const double quantile = boost::math::quantile(skew_distribution, uniform(threadContext.rng));
		HitPoints damage = quantile * 4 * variability + weapon_damage;

		for (int i = 0; i < defense; ++i) {
			if (damage == 0)
				break;

			if (defense_distribution(threadContext.rng) < 10 * defender_luck)
				--damage;
		}

		return damage;
	}
}
