#pragma once
#include "config.h"
#ifdef ENABLE_ZIP8
#include "zip8.h"

#include <sigc++/sigc++.h>

#include <chrono>
#include <span>
#include <string>

namespace Game3 {
	class Zip8 {
		public:
			sigc::signal<void(std::span<const uint8_t>)> signalDisplayUpdated;

			Zip8(std::string_view program, uint64_t seed = 0);
			~Zip8();

			void tick(std::size_t cycles = 100'000);

		private:
			void *cpu = nullptr;
			uint16_t error{};
			std::chrono::system_clock::time_point lastTimerUpdate{};
	};
}

#endif
