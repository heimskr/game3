#include "ui/MiniAudio.h"
#include "ui/Sound.h"

namespace Game3 {
	SoundEngine::SoundEngine() {
		if (auto result = MiniAudio::makeEngine(engine, resourceManager); result != MA_SUCCESS)
			throw MiniAudio::AudioError("Failed to initialize miniaudio engine", result);
	}

	SoundEngine::~SoundEngine() {
		ma_engine_uninit(&engine);
	}

	void SoundEngine::play(const std::filesystem::path &path) {
		if (auto result = ma_engine_play_sound(&engine, path.c_str(), nullptr); result != MA_SUCCESS)
			throw MiniAudio::AudioError("Failed to play sound", result);
	}
}
