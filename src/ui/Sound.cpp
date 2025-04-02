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

	void SoundEngine::cleanup() {
		impl->cleanup();
	}

	SoundEngineImpl::SoundEngineImpl() {
		if (auto result = MiniAudio::makeEngine(engine, resourceManager, &vtables); result != MA_SUCCESS) {
			throw MiniAudio::AudioError("Failed to initialize miniaudio engine", result);
		}

		decoderConfig = ma_decoder_config_init_default();
		decoderConfig.ppCustomBackendVTables = vtables;
		decoderConfig.customBackendCount = 2;
	}

	SoundEngineImpl::~SoundEngineImpl() {
		ma_engine_uninit(&engine);
		ma_resource_manager_uninit(&resourceManager);
	}

	void SoundEngineImpl::play(const std::filesystem::path &path, float pitch) {
		auto lock = uniqueLock();
		ma_sound &sound = getSound(path, pitch);
		ma_sound_start(&sound);
	}

	void SoundEngineImpl::cleanup() {
		auto lock = uniqueLock();

		std::erase_if(sounds, [&](auto &item) {
			auto &[pair, sound] = item;
			if (ma_sound_is_playing(&sound)) {
				return false;
			}
			ma_sound_uninit(&sound);
			return true;
		});

		std::erase_if(decoders, [&](auto &pair) {
			return !pair.second.isBusy();
		});
	}

	ma_sound & SoundEngineImpl::getSound(const std::filesystem::path &path, float pitch) {
		std::pair pair{path, pitch};

		for (auto [iter, end] = sounds.equal_range(pair); iter != end; ++iter) {
			ma_sound &sound = iter->second;
			if (!ma_sound_is_playing(&sound)) {
				return sound;
			}
		}

		auto iter = sounds.emplace(pair, ma_sound{});
		ma_sound &sound = iter->second;

		MiniAudio::Decoder &decoder = getDecoder(path);

		if (auto result = ma_sound_init_from_data_source(&engine, decoder, 0, nullptr, &sound); result != MA_SUCCESS) {
			sounds.erase(iter);
			throw MiniAudio::AudioError("Failed to initialize sound", result);
		}

		if (pitch != 1.f) {
			ma_sound_set_pitch(&sound, pitch);
		}

		return sound;
	}

	MiniAudio::Decoder & SoundEngineImpl::getDecoder(const std::filesystem::path &path) {
		MiniAudio::Decoder *decoder = nullptr;

		for (auto [iter, end] = decoders.equal_range(path); iter != end; ++iter) {
			if (!iter->second.isBusy()) {
				decoder = &iter->second;
				break;
			}
		}

		if (!decoder) {
			auto new_iter = decoders.emplace(path, readFile(path));
			decoder = &new_iter->second;
		}

		assert(decoder);

		decoder->init(decoderConfig);
		return *decoder;
	}
}
