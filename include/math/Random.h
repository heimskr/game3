#pragma once

#include <random>

namespace Game3 {
	struct Vector3;
	Vector3 randomOnSphere(std::default_random_engine &);
	Vector3 randomOnSphereUpperHalf(std::default_random_engine &);
}
