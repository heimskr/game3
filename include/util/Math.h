#pragma once

#include <cmath>
#include <type_traits>

namespace Game3 {
	template <typename T>
	inline T updiv(T n, std::type_identity_t<T> d) {
		return n / d + (n % d? 1 : 0);
	}

	inline double fractional(double d) {
		return std::modf(d, &d);
	}
}
