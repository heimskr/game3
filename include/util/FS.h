#pragma once

#include <filesystem>
#include <string>

namespace Game3 {
	extern std::filesystem::path dataRoot;

	std::string readFile(const std::filesystem::path &);
	bool isSubpath(const std::filesystem::path &base, std::filesystem::path to_check);
}
