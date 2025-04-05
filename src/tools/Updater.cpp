#include "tools/Updater.h"
#include "util/FS.h"
#include "util/Log.h"
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

	void Updater::updateFetch(const std::string &domain) {
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

		updateLocal(std::move(string_stream).str());
	}

	void Updater::updateLocal(std::string raw_zip) {
		std::filesystem::path directory = "./update";
		Zip(std::move(raw_zip)).unzipTo(directory);

#ifdef __MINGW32__
		std::filesystem::path pdb = directory / "Game3" / "game3.pdb";
		if (std::filesystem::exists(pdb)) {
			std::filesystem::rename(pdb, "./game3.pdb");
		}

		std::filesystem::path cwd = std::filesystem::current_path();

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

		std::filesystem::remove_all(directory);
	}

	void Updater::update() {
		updateFetch(DEFAULT_DOMAIN);
	}
}
