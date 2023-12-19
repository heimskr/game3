#include "graphics/Multiplier.h"
#include "util/FS.h"

namespace Game3 {
	namespace {
		const std::string & multiplierFrag() { static auto out = readFile("resources/multiplier.frag"); return out; }
		const std::string & multiplierVert() { static auto out = readFile("resources/multiplier.vert"); return out; }
	}

	Multiplier::Multiplier():
		TextureCombiner("multiplier", multiplierVert(), multiplierFrag()) {}
}
