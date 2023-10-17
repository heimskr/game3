#include "types/Types.h"

namespace Game3 {
	Index operator""_idx(unsigned long long value) {
		return static_cast<Index>(value);
	}

	std::ostream & operator<<(std::ostream &os, PipeType pipe_type) {
		switch (pipe_type) {
			case PipeType::Item:   return os << "Item";
			case PipeType::Fluid:  return os << "Fluid";
			case PipeType::Energy: return os << "Energy";
			default:
				return os << "Invalid";
		}
	}
}
