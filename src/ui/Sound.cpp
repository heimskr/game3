#include "util/Log.h"
#include "ui/MiniAudio.h"
#include "ui/Sound.h"
#include "ui/SoundImpl.h"
#include "util/FS.h"

namespace Game3 {
	SoundEngine::SoundEngine():
		impl(std::make_unique<SoundEngineImpl>()) {}

	SoundEngine::~SoundEngine() = default;

	void SoundEngine::play(const std::filesystem::path &path, float pitch) {
		impl->play(path, pitch);
	}

	size_t SoundEngine::cleanup() {
		return impl->cleanup();
	}

	SoundEngineImpl::SoundEngineImpl() {
		if (auto result = MiniAudio::makeEngine(engine, resourceManager, &vtables); result != MA_SUCCESS)
			throw MiniAudio::AudioError("Failed to initialize miniaudio engine", result);

		decoderConfig = ma_decoder_config_init_default();
		decoderConfig.ppCustomBackendVTables = vtables;
		decoderConfig.customBackendCount = 2;
	}

	SoundEngineImpl::~SoundEngineImpl() {
		ma_engine_uninit(&engine);
		ma_resource_manager_uninit(&resourceManager);
	}

	void SoundEngineImpl::play(const std::filesystem::path &path, float pitch) {
		ma_sound &sound = getSound(path, pitch);
		ma_sound_start(&sound);
	}

	size_t SoundEngineImpl::cleanup() {
		size_t count = 0;

		std::erase_if(sounds, [&](auto &item) {
			auto &[pair, sound] = item;
			if (ma_sound_is_playing(&sound))
				return false;
			++count;
			ma_sound_uninit(&sound);
			return true;
		});

		return count;
	}

	ma_sound & SoundEngineImpl::getSound(const std::filesystem::path &path, float pitch) {
		std::pair pair{path, pitch};
		auto [begin, end] = sounds.equal_range(pair);

		for (auto iter = begin; iter != end; ++iter) {
			ma_sound &sound = iter->second;
			if (!ma_sound_is_playing(&sound))
				return sound;
		}

		auto iter = sounds.emplace(pair, ma_sound{});
		ma_sound &sound = iter->second;

		ma_decoder &decoder = getDecoder(path);

		if (auto result = ma_sound_init_from_data_source(&engine, &decoder, 0, nullptr, &sound); result != MA_SUCCESS) {
			sounds.erase(iter);
			throw MiniAudio::AudioError("Failed to initialize sound", result);
		}

		if (pitch != 1.f) {
			ma_sound_set_pitch(&sound, pitch);
		}

		return sound;
	}

	ma_decoder & SoundEngineImpl::getDecoder(const std::filesystem::path &path) {
		std::pair<ma_decoder, std::string> *pair{};

		if (auto iter = decoders.find(path); iter != decoders.end()) {
			pair = &iter->second;
		} else {
			auto [new_iter, inserted] = decoders.emplace(path, std::pair{ma_decoder{}, readFile(path)});
			assert(inserted);
			pair = &new_iter->second;
		}

		assert(pair);
		auto &[decoder, data] = *pair;

		if (auto result = ma_decoder_init_memory(data.data(), data.size(), &decoderConfig, &decoder); result != MA_SUCCESS) {
			throw MiniAudio::AudioError("Failed to reinitialize decoder", result);
		}

		return decoder;
	}
}
