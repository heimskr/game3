#pragma once

#include "threading/HasMutex.h"

#include <miniaudio/miniaudio.h>

#include <filesystem>
#include <map>

namespace Game3 {
	class SoundEngineImpl final: public HasMutex<SoundEngineImpl> {
		public:
			SoundEngineImpl();
			~SoundEngineImpl();

			void play(const std::filesystem::path &, float pitch, float volume);
			void cleanup();

		private:
			std::multimap<std::pair<std::filesystem::path, float>, ma_sound> sounds;
			std::multimap<std::filesystem::path, MiniAudio::Decoder> decoders;
			ma_resource_manager resourceManager{};
			ma_engine engine{};
			ma_decoding_backend_vtable * vtables[2]{};
			ma_decoder_config decoderConfig{};

			/** Doesn't lock. */
			ma_sound & getSound(const std::filesystem::path &, float pitch = 1);

			/** Doesn't lock. */
			MiniAudio::Decoder & getDecoder(const std::filesystem::path &);
	};
}
