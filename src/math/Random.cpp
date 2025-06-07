#include "math/Random.h"
#include "math/Vector.h"

#include <cmath>

namespace Game3 {
	Vector3 randomOnSphere(std::default_random_engine &rng) {
		// https://math.stackexchange.com/a/1586185
		std::uniform_real_distribution distribution(0.0, 1.0);
		const double u1 = distribution(rng);
		const double u2 = distribution(rng);
		const double phi = std::acos(2 * u1 - 1) - 3.14159265358979323846 / 2;
		const double lambda = 2.0 * 3.14159265358979323846 * u2;
		const double cos_phi = std::cos(phi);
		return {
			cos_phi * std::cos(lambda),
			cos_phi * std::sin(lambda),
			std::sin(phi),
		};
	}

	Vector3 randomOnSphereUpperHalf(std::default_random_engine &rng) {
		Vector3 vector = randomOnSphere(rng);
		vector.y = std::abs(vector.y);
		return vector;
	}
}
