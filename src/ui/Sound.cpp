#include "ui/Sound.h"

namespace Game3 {
	Sound::Sound(const std::filesystem::path &path):
		mediaFile(Gtk::MediaFile::create_for_filename(path.string())) {}

	void Sound::play() {
		sought = false;
		mediaFile->play();
		lastPlayed = std::chrono::system_clock::now();
	}

	bool Sound::isReady() {
		if (mediaFile->get_playing())
			return false;

		if (sought)
			return true;

		if (seekStarted) {
			if (mediaFile->is_seeking())
				return false;

			seekStarted = false;
			sought = true;
			return true;
		}

		seekStarted = true;
		mediaFile->seek(0);
		return false;
	}

	SoundProvider::SoundProvider() {
		soloud.init();
	}

	SoundProvider::~SoundProvider() {
		soloud.deinit();
	}

	void SoundProvider::play(const std::filesystem::path &path) {
		auto lock = soundMap.uniqueLock();
		auto [iter, inserted] = soundMap.try_emplace(path);
		SoLoud::Wav &sound = iter->second;

		if (inserted)
			sound.load(path.c_str());

		soloud.play(sound);
	}
}
