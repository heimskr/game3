#include "Types.h"

namespace Game3 {
	Index operator""_idx(unsigned long long value) {
		return static_cast<Index>(value);
	}
}
