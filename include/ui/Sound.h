#pragma once

#include "threading/Lockable.h"

#include <gtkmm.h>
#include <soloud/soloud.h>
#include <soloud/soloud_wav.h>

#include <chrono>
#include <filesystem>
#include <map>
#include <vector>

namespace Game3 {
	class Sound {
		public:
			Sound(const std::filesystem::path &);

			void play();
			bool isReady();

		private:
			bool sought = true;
			bool seekStarted = false;
			Glib::RefPtr<Gtk::MediaFile> mediaFile;
			std::chrono::system_clock::time_point lastPlayed;

		friend class SoundProvider;
	};

	class SoundProvider {
		public:
			SoundProvider();
			~SoundProvider();

			void play(const std::filesystem::path &);

		private:
			Lockable<std::map<std::filesystem::path, SoLoud::Wav>> soundMap;

			// MAKE IT STOP
			SoLoud::Soloud soloud;
	};
}
