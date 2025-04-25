#include "data/SoundSet.h"
#include "threading/ThreadContext.h"
#include "util/Util.h"

namespace Game3 {
	SoundSet::SoundSet(Identifier identifier, Set sounds, float pitchVariance):
		NamedRegisterable(std::move(identifier)),
		sounds(std::move(sounds)),
		pitchVariance(pitchVariance) {}

	const Identifier & SoundSet::choose() const {
		return Game3::choose(sounds, threadContext.rng);
	}

	float SoundSet::choosePitch() const {
		return threadContext.getPitch(pitchVariance);
	}
}
