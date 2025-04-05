#pragma once

#include <utility>

namespace Game3 {
	// Credit: https://stackoverflow.com/a/55083395
	template <typename T, typename U>
	struct PairHash {
		static size_t operator()(const std::pair<T, U> &pair) noexcept {
			uintmax_t hash = std::hash<T>{}(pair.first);
			hash <<= sizeof(uintmax_t) * 4;
			hash ^= std::hash<U>{}(pair.second);
			return std::hash<uintmax_t>{}(hash);
		}
	};
}
