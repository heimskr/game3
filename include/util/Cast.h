#pragma once

#include "util/Demangle.h"

#include <memory>

namespace Game3 {
	template <typename T, typename S>
	std::shared_ptr<T> safeDynamicCast(const std::shared_ptr<S> &source) {
		if (auto cast = std::dynamic_pointer_cast<T>(source))
			return cast;
		throw std::invalid_argument("Dynamic cast from " + DEMANGLE(S) + " to " + DEMANGLE(T) + " failed");
	}
}