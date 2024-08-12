#pragma once

#define MA_NO_PTHREAD_INCLUDE
#include <thread>
#include <miniaudio/miniaudio.h>

#include <stdexcept>
#include <string>

namespace MiniAudio {
	struct AudioError: std::runtime_error {
		ma_result result{};
		AudioError(const std::string &, ma_result);
		AudioError(ma_result);
	};

	ma_result makeEngine(ma_engine &, ma_resource_manager &, ma_decoding_backend_vtable * (*vtables)[2] = nullptr);
}
