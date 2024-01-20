#pragma once

#include <cstdint>

namespace Game3 {
	constexpr static int64_t SERVER_TICK_FREQUENCY = 20;
	constexpr static int64_t SERVER_TICK_PERIOD = 1000 / SERVER_TICK_FREQUENCY;

	constexpr static int64_t CLIENT_TICK_FREQUENCY = 144;
	constexpr static int64_t CLIENT_TICK_PERIOD = 1000 / CLIENT_TICK_FREQUENCY;
}
