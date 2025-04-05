#include "tools/Updater.h"
#include "util/FS.h"
#include "util/Log.h"
#include "util/PairHash.h"
#include "util/VectorHash.h"
#include "util/Zip.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#pragma GCC diagnostic pop

#include <format>
#include <sstream>

namespace {
	std::string DEFAULT_DOMAIN = "game3.zip";
#ifdef __MINGW32__
	std::string EXECUTABLE = "game3.exe";
	std::string TEMP_EXECUTABLE = "./graveyard/game3.exe";
#else
	std::string EXECUTABLE = "game3";
	std::string TEMP_EXECUTABLE = "./_game3";
#endif

	static void maybeInitializeCurl() {
		static bool initialized = false;
		if (!initialized) {
			curlpp::initialize();
			initialized = true;
		}
	}
}

namespace Game3 {
	Updater::Updater() = default;

	bool Updater::updateFetch(const std::string &domain) {
		maybeInitializeCurl();

		std::string url;
#ifdef __MINGW32__
		url = std::format("https://{}/game3-windows-x86_64.zip", domain);
#elif defined(__linux__)
		url = std::format("https://{}/game3-linux-x86_64.zip", domain);
#else
		throw std::runtime_error("Can't update: unknown platform");
#endif

		curlpp::Easy request;
		std::ostringstream string_stream;
		request.setOpt(curlpp::options::Url(url));
		request.setOpt(curlpp::options::WriteStream(&string_stream));
		request.setOpt(curlpp::options::FailOnError(true));
#ifdef __MINGW32__
		request.setOpt(curlpp::options::SslOptions(CURLSSLOPT_NATIVE_CA));
#endif
		request.perform();

		return updateLocal(std::move(string_stream).str());
	}

	bool Updater::updateLocal(std::string raw_zip) {
		std::filesystem::path directory = "./update";
		Zip(std::move(raw_zip)).unzipTo(directory);

		if (!mayUpdate()) {
			WARN("Refusing to install update because meson.build exists.");
			return false;
		}

		std::filesystem::path cwd = std::filesystem::current_path();

#ifdef __MINGW32__
		std::filesystem::path pdb = directory / "Game3" / "game3.pdb";
		if (std::filesystem::exists(pdb)) {
			std::filesystem::rename(pdb, "./game3.pdb");
		}

		std::filesystem::path graveyard = cwd / "graveyard";
		if (!std::filesystem::exists(graveyard)) {
			std::filesystem::create_directory(graveyard);
		}

		if (!std::filesystem::is_symlink(EXECUTABLE)) {
			std::filesystem::rename("game3.exe", graveyard / "game3.exe");
			std::filesystem::rename(directory / "Game3" / "game3.exe", "game3.exe");
		}

		for (const std::filesystem::directory_entry &entry: std::filesystem::directory_iterator(directory / "Game3")) {
			const std::filesystem::path &path = entry.path();
			if (path.extension() == ".dll") {
				std::filesystem::path old_path = cwd / path.filename();
				std::filesystem::rename(old_path, graveyard / path.filename().string());
				std::filesystem::rename(entry.path(), old_path);
			}
		}
#else
		if (!std::filesystem::is_symlink(EXECUTABLE)) {
			std::filesystem::rename(EXECUTABLE, TEMP_EXECUTABLE);
			std::filesystem::rename(directory / "Game3" / EXECUTABLE, EXECUTABLE);
			markExecutable(EXECUTABLE);
		}
#endif

		std::filesystem::remove_all("./resources");
		std::filesystem::rename(directory / "Game3" / "resources", "./resources");

		std::filesystem::remove_all("./gamedata");
		std::filesystem::rename(directory / "Game3" / "gamedata", "./gamedata");

		for (std::string_view subdir: {"src", "include"}) {
			std::filesystem::path in_update = directory / "Game3" / subdir;
			if (std::filesystem::exists(in_update)) {
				std::filesystem::path in_root = cwd / subdir;
				if (std::filesystem::exists(in_root)) {
					std::filesystem::remove_all(in_root);
				}
				std::filesystem::rename(in_update, in_root);
			}
		}

		std::filesystem::remove_all(directory);
		return true;
	}

	bool Updater::update() {
		return updateFetch(DEFAULT_DOMAIN);
	}

	bool Updater::mayUpdate() {
		return !std::filesystem::exists("meson.build");
	}

	std::size_t Updater::getLocalHash() {
		std::map<std::string, std::size_t> hash_map; // haha. funny
		const std::filesystem::path cwd = std::filesystem::current_path();

		auto absorb = [&](this const auto &absorb, const std::filesystem::path &path) -> void {
			if (std::filesystem::is_directory(path)) {
				for (const std::filesystem::directory_entry &entry: std::filesystem::directory_iterator(path)) {
					absorb(entry.path());
				}
				return;
			}
			std::filesystem::path relative = std::filesystem::relative(path, cwd);
			std::size_t hash = std::hash<std::string>{}(readFile(relative));
			hash_map[std::move(relative)] = hash;
		};

		absorb(cwd / EXECUTABLE);
		absorb(cwd / "resources");

		std::vector<std::size_t> hash_vector;
		hash_vector.reserve(hash_map.size());

		for (const auto &[path, hash]: hash_map) {
			hash_vector.emplace_back(hash);
		}

		return std::hash<decltype(hash_vector)>{}(hash_vector);
	}
}
