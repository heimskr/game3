#pragma once

#include <miniaudio/miniaudio.h>

#include <stdexcept>
#include <string>

namespace MiniAudio {
	struct AudioError: std::runtime_error {
		ma_result result{};
		AudioError(const std::string &, ma_result);
		AudioError(ma_result);
	};

	ma_result makeEngine(ma_engine &, ma_resource_manager &);
}
