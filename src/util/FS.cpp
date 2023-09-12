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
		std::ifstream stream;
		stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		stream.open(path);
		stream.exceptions(std::ifstream::goodbit);
		if (!stream.is_open())
			throw std::runtime_error("Couldn't open file for reading");
		stream.seekg(0, std::ios::end);
		std::string out;
		out.reserve(stream.tellg());
		stream.seekg(0, std::ios::beg);
		out.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
		stream.close();
		return out;
	}
}
