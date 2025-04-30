#include "data/SoundSet.h"
#include "game/Game.h"
#include "mixin/HasSoundSet.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"

namespace Game3 {
	HasSoundSet::HasSoundSet(Identifier soundSetID):
		soundSetID(std::move(soundSetID)) {}

	const std::shared_ptr<SoundSet> & HasSoundSet::getSoundSet() {
		if (!soundSet && soundSetID) {
			soundSet = getGame()->registry<SoundSetRegistry>().at(soundSetID);
		}

		return soundSet;
	}

	void HasSoundSet::encodeSoundSet(Buffer &buffer) const {
		if (soundSet) {
			buffer << soundSet->identifier;
		} else {
			buffer << soundSetID;
		}
	}

	void HasSoundSet::decodeSoundSet(Buffer &buffer) {
		buffer >> soundSetID;
		soundSet.reset();
	}


	void HasSoundSet::playSound(const Place &place) {
		if (const SoundSetPtr &sound_set = getSoundSet()) {
			float pitch = 1;
			if (float variance = sound_set->pitchVariance; variance != 1) {
				pitch = threadContext.getPitch(variance);
			}

			place.realm->playSound(place.position, sound_set->choose(), pitch, 64);
		}
	}
}
