#include "config.h"
#include "util/FS.h"

#include <fstream>

namespace Game3 {
	std::filesystem::path dataRoot =
#ifdef IS_FLATPAK
		"/app/bin";
#else
		".";
#endif

	std::string readFile(const std::filesystem::path &path) {
		if (std::filesystem::is_directory(path)) {
			throw std::runtime_error("Can't read directory");
		}

		std::ifstream stream;
		stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		stream.open(path);
		stream.exceptions(std::ifstream::goodbit);

		if (!stream.is_open()) {
			throw std::ios_base::failure("Couldn't open file for reading");
		}

		stream.seekg(0, std::ios::end);
		std::string out;
		out.reserve(stream.tellg());
		stream.seekg(0, std::ios::beg);
		out.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
		stream.close();
		return out;
	}

	bool isSubpath(const std::filesystem::path &base, std::filesystem::path to_check) {
		if (base == to_check) {
			return true;
		}

		const std::filesystem::path root("/");

		while (to_check != root && !to_check.empty()) {
			if (to_check == base) {
				return true;
			}

			to_check = to_check.parent_path();
		}

		return false;
	}
}
