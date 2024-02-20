#pragma once

#include "threading/Lockable.h"

#include <gtkmm.h>
#include <miniaudio/miniaudio.h>

#include <chrono>
#include <filesystem>
#include <map>
#include <vector>

namespace Game3 {
	class SoundEngine {
		public:
			SoundEngine();
			~SoundEngine();

			void play(const std::filesystem::path &);

		private:
			ma_resource_manager resourceManager;
			ma_engine engine;
	};
}
