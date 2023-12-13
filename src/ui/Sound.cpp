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

	SoundProvider::SoundProvider() = default;

	void SoundProvider::play(const std::filesystem::path &path) {
		auto lock = soundMap.uniqueLock();
		std::vector<Sound> &sounds = soundMap[path];

		if (Sound *sound = findNotPlaying(sounds)) {
			sound->play();
			return;
		}

		sounds.emplace_back(path);
		sounds.back().play();
	}

	Sound * SoundProvider::findNotPlaying(std::vector<Sound> &sounds) {
		for (Sound &sound: sounds)
			if (sound.isReady())
				return &sound;
		return nullptr;
	}
}
