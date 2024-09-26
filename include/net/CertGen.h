#pragma once

#include <filesystem>

namespace Game3 {
	void generateCertPair(const std::filesystem::path &certificate_path, const std::filesystem::path &key_path);
}
