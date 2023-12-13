#include "ui/Sound.h"

namespace Game3 {
	Sound::Sound(const std::filesystem::path &path):
		mediaFile(Gtk::MediaFile::create_for_filename(path.string())) {}

	void Sound::play() {
		mediaFile->play();
	}

	bool Sound::isReady() {
		return !mediaFile->get_playing();
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
