#pragma once

#include "registry/Registry.h"

#include <unordered_set>

namespace Game3 {
	struct SoundSet: public NamedRegisterable {
		using Set = std::unordered_set<Identifier>;
		Set sounds;
		float pitchVariance;
		SoundSet(Identifier identifier, Set sounds, float pitchVariance = 1);
		const Identifier & choose() const;
		float choosePitch() const;
	};

	using SoundSetPtr = std::shared_ptr<SoundSet>;

	struct SoundSetRegistry: NamedRegistry<SoundSet> {
		static Identifier ID() { return {"base", "registry/sound_set"}; }
		SoundSetRegistry(): NamedRegistry(ID()) {}
	};
}
