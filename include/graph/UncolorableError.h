#pragma once

#include <stdexcept>

namespace Game3 {
	struct UncolorableError: std::runtime_error {
		UncolorableError(): std::runtime_error("Unable to color graph: not enough colors") {}
	};
}
