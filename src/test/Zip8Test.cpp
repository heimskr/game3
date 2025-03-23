#include "config.h"

#ifdef ENABLE_ZIP8

#include "util/Log.h"
#include "util/FS.h"
#include "util/Util.h"
#include "zip8.h"

#include <chrono>
#include <cstdlib>

extern "C" void zip8Log(const char *, size_t) {
	// Game3::INFO("Zip8: {}", std::string_view(buf, len));
}

namespace Game3 {
	void zip8Test() {
		void *cpu = std::malloc(zip8CpuGetSize());
		uint16_t err{};

		std::string flappy = readFile("chip8/flappyai.ch8");

		zip8CpuInit(&err, cpu, reinterpret_cast<const uint8_t *>(flappy.data()), flappy.size(), 64, 0);

		if (err != 0) {
			throw std::runtime_error(std::format("Couldn't init Zip8: {}", err));
		}

		auto last_time = getTime();

		for (;;) {
			if (zip8CpuDisplayIsDirty(cpu)) {
				const uint8_t *display = zip8CpuGetDisplay(cpu);
				zip8CpuSetDisplayNotDirty(cpu);

				std::print("\x1b[3J\x1b[H");

				for (int row = 0; row < 32; ++row) {
					for (int col = 0; col < 8; ++col) {
						uint8_t pack = display[row * 8 + col];
						for (int i = 0; i < 8; ++i) {
							bool pixel = pack & 1;
							pack >>= 1;
							std::cout << (pixel? 'X' : '_');
						}
					}
					std::cout << '\n';
				}
			}

			zip8CpuCycle(&err, cpu);

			auto now = getTime();

			if ((now - last_time).count() / 1e9 > 1.0 / 60.0) {
				last_time = now;
				zip8CpuTimerTick(cpu);
			}

			if (err != 0) {
				throw std::runtime_error(std::format("Failed to tick CPU: {}", err));
			}
		}
	}
}

#endif
