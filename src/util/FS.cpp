#include "config.h"
#include "util/FS.h"
#include "util/Log.h"

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
		stream.open(path, std::ios_base::in | std::ios_base::binary);
		stream.exceptions(std::ifstream::goodbit);

		if (!stream.is_open()) {
			throw std::ios_base::failure("Couldn't open file for reading");
		}

		stream.seekg(0, std::ios::end);
		std::string out;
		out.reserve(stream.tellg());
		stream.seekg(0, std::ios::beg);
#ifdef __APPLE__
		// On My Machine™, the readsome trick just gives me an empty string.
		// My apologies to the 0 people who play game3 on macOS.
		out.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
#else
		std::array<char, 4096> buffer;

		std::streamsize bytes_read = 0;
		while ((bytes_read = stream.readsome(buffer.data(), buffer.size())) > 0) {
			out.append(buffer.data(), bytes_read);
		}
#endif
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

	void markExecutable(const std::filesystem::path &path) {
		std::filesystem::perms perms = std::filesystem::status(path).permissions();
		perms |= std::filesystem::perms::owner_exec;
		std::filesystem::permissions(path, perms);
	}

	bool canListDirectory(const std::filesystem::path &path) {
		std::error_code code{};
		std::filesystem::directory_iterator(path, code);
		return !code;
	}
}
