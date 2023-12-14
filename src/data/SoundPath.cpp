#include "data/SoundPath.h"

namespace Game3 {
	SoundPath::SoundPath(Identifier identifier_, std::filesystem::path path_):
		NamedRegisterable(std::move(identifier_)), path(std::move(path_)) {}
}
