#pragma once

#include "threading/Lockable.h"

#include <gtkmm.h>

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
			Glib::RefPtr<Gtk::MediaFile> mediaFile;
			std::chrono::system_clock::time_point lastPlayed;
	};

	class SoundProvider {
		public:
			SoundProvider();

			void play(const std::filesystem::path &);

		private:
			Lockable<std::map<std::filesystem::path, std::vector<Sound>>> soundMap;

			static Sound * findNotPlaying(std::vector<Sound> &);
	};
}
