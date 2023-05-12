#pragma once

#include <string>

namespace Game3 {
	template <typename T>
	T computeSHA3(std::string_view input);

	/** Returns a secret hex string of a variable length. The length will be in the range [count, 16 * count] (not an even distribution). */
	std::string generateSecret(size_t count);
}
