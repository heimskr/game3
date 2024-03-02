#pragma once

#include "threading/Lockable.h"

#include <miniaudio/miniaudio.h>

#include <filesystem>
#include <map>

namespace Game3 {
	class SoundEngineImpl {
		public:
			SoundEngineImpl();
			~SoundEngineImpl();

			void play(const std::filesystem::path &, float pitch);
			size_t cleanup();

		private:
			std::multimap<std::pair<std::filesystem::path, float>, ma_sound> sounds;
			std::map<std::filesystem::path, std::pair<ma_decoder, std::string>> decoders;
			ma_resource_manager resourceManager;
			ma_engine engine;
			ma_decoding_backend_vtable * vtables[2];
			ma_decoder_config decoderConfig;

			ma_sound & getSound(const std::filesystem::path &, float pitch = 1.f);
			ma_decoder & getDecoder(const std::filesystem::path &);
	};
}
