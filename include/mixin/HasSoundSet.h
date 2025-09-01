#pragma once

#include "data/Identifier.h"

#include <memory>

namespace Game3 {
	class BasicBuffer;
	class Buffer;
	class Game;
	struct Place;
	struct SoundSet;

	class HasSoundSet {
		public:
			HasSoundSet() = default;
			HasSoundSet(Identifier soundSetID);

		protected:
			std::shared_ptr<SoundSet> soundSet;
			Identifier soundSetID;

			virtual std::shared_ptr<Game> getGame() const = 0;
			const std::shared_ptr<SoundSet> & getSoundSet();
			void encodeSoundSet(Buffer &) const;
			void decodeSoundSet(Buffer &);
			void playSound(const Place &);
	};
}
