#pragma once

#include <chrono>

namespace Game3 {
	struct PackedTime {
		uint64_t milliseconds{};

		PackedTime(uint64_t milliseconds):
			milliseconds(milliseconds) {}

		PackedTime(std::chrono::system_clock::time_point time):
			milliseconds(std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count()) {}

		inline operator std::chrono::system_clock::time_point() const {
			return std::chrono::system_clock::time_point(std::chrono::milliseconds(milliseconds));
		}

		inline auto timeSince() const {
			return std::chrono::system_clock::now() - static_cast<std::chrono::system_clock::time_point>(*this);
		}

		static inline PackedTime now() {
			return PackedTime(std::chrono::system_clock::now());
		}
	};
}
