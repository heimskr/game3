#pragma once

#include <concepts>

namespace Game3 {
	template <typename T>
	concept Numeric = std::integral<T> || std::floating_point<T>;
}
