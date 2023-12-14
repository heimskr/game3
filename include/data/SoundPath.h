#pragma once

#include "registry/Registerable.h"

#include <filesystem>

namespace Game3 {
	struct SoundPath: NamedRegisterable {
		std::filesystem::path path;
		SoundPath(Identifier, std::filesystem::path);
	};
}
