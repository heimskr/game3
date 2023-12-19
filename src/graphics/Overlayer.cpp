#include "graphics/Overlayer.h"
#include "util/FS.h"

namespace Game3 {
	namespace {
		const std::string & overlayerFrag() { static auto out = readFile("resources/overlayer.frag"); return out; }
		const std::string & overlayerVert() { static auto out = readFile("resources/overlayer.vert"); return out; }
	}

	Overlayer::Overlayer():
		TextureCombiner("overlayer", overlayerVert(), overlayerFrag()) {}
}
