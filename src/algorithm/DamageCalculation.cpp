#include "algorithm/DamageCalculation.h"
#include "threading/ThreadContext.h"

#include <boost/math/distributions/skew_normal.hpp>

namespace Game3 {
	HitPoints calculateDamage(HitPoints weapon_damage, int variability, double attacker_luck) {
		boost::math::skew_normal_distribution<double> skew_distribution(0, 0.15, attacker_luck); // With luck = 0, this ranges from roughly -0.25 to 0.25
		std::uniform_real_distribution<double> uniform(0., 1.);
		const double quantile = boost::math::quantile(skew_distribution, uniform(threadContext.rng));
		return quantile * 4 * variability + weapon_damage;
	}
}
