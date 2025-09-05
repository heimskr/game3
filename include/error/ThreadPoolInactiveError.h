#pragma once

#include <stdexcept>

namespace Game3 {
	struct ThreadPoolInactiveError: std::runtime_error {
		using std::runtime_error::runtime_error;
	};
}
