#pragma once

#include <format>
#include <stdexcept>

namespace Game3 {
	struct IncompatibleError: std::runtime_error {
		int64_t compatibility{};

		IncompatibleError(int64_t compatibility):
			std::runtime_error(getMessage(compatibility)),
			compatibility(compatibility) {}

		static std::string getMessage(int64_t compatibility) {
			if (compatibility == 0) {
				return "Current format version is the same as the saved format version but it's incompatible anyway???";
			} else if (compatibility < 0) {
				return std::format("Current format version differs from saved format version by {} (save is too old)", compatibility);
			} else {
				return std::format("Current format version differs from saved format version by {} (save is too new)", compatibility);
			}
		}
	};
}
