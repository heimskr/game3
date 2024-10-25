#include "config.h"
#ifdef ENABLE_ZIP8
#include "lib/Chip8.h"

#include <cstdlib>
#include <format>
#include <stdexcept>

namespace Game3 {
	Zip8::Zip8(std::string_view program, uint64_t seed):
		cpu(std::malloc(zip8CpuGetSize())) {
			zip8CpuInit(&error, cpu, reinterpret_cast<const uint8_t *>(program.data()), program.size(), seed, 0);
			if (error != 0) {
				throw std::runtime_error(std::format("Couldn't init Zip8: {}", error));
			}
		}

	Zip8::~Zip8() {
		std::free(cpu);
	}

	void Zip8::tick(std::size_t cycles) {
		if (zip8CpuDisplayIsDirty(cpu)) {
			signalDisplayUpdated(std::span(zip8CpuGetDisplay(cpu), 64 * 32 / 8));
			zip8CpuSetDisplayNotDirty(cpu);
		}

		for (std::size_t cycle = 0; cycle < cycles; ++cycle) {
			zip8CpuCycle(&error, cpu);
			if (error != 0) {
				throw std::runtime_error(std::format("Failed to tick CPU: {}", error));
			}
		}

		auto now = std::chrono::system_clock::now();
		if ((now - lastTimerUpdate).count() > 1'000'000'000 / 60) {
			zip8CpuTimerTick(cpu);
			lastTimerUpdate = now;
		}
	}

	void Zip8::setKey(uint8_t key, bool value) {
		if (value) {
			setKeys(keys | (1 << key));
		} else {
			setKeys(keys & ~(1 << key));
		}
	}

	void Zip8::setKeys(uint16_t new_keys) {
		if (keys != new_keys) {
			keys = new_keys;
			zip8CpuSetKeys(cpu, keys);
		}
	}
}

#endif
