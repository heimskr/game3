#pragma once

#include <stdexcept>
#include <string>

namespace Game3 {
	struct OverlapError: std::runtime_error {
		std::string prefix;

		OverlapError(const std::string &prefix_):
			std::runtime_error(prefix_ + ": overlap would occur"), prefix(prefix_) {}
	};
}
