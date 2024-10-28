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
			void setKey(uint8_t key, bool);
			void setKeys(uint16_t);
			void * getCPU() const;
			uint64_t getFlags() const;
			bool getFlagsDirty() const;
			void clearFlagsDirty();

		private:
			void *cpu = nullptr;
			uint16_t error{};
			std::chrono::system_clock::time_point lastTimerUpdate{};
			uint16_t keys{};
	};
}

#endif
