#pragma once

#include <filesystem>
#include <memory>

namespace Game3 {
	class SoundEngineImpl;

	class SoundEngine {
		public:
			SoundEngine();
			~SoundEngine();

			void play(const std::filesystem::path &, float pitch = 1.f);
			void cleanup();

		private:
			std::unique_ptr<SoundEngineImpl> impl;
	};
}
