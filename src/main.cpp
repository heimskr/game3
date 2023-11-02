#include "config.h"
#include "App.h"
#include "Log.h"
#include "client/RichPresence.h"
#include "net/Server.h"
#include "net/Sock.h"
#include "tools/Flasker.h"
#include "tools/ItemStitcher.h"
#include "tools/Mazer.h"
#include "tools/Migrator.h"
#include "tools/TileStitcher.h"
#include "util/Crypto.h"
#include "util/FS.h"
#include "util/Timer.h"
#include "util/Util.h"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <vector>

#include <GLFW/glfw3.h>

namespace Game3 {
	void test();
	void splitter();
	void omniOptOut();
	void filterTest();
	bool chemskrTest(int, char **);
}

int main(int argc, char **argv) {
#ifdef IS_FLATPAK
	if (const char *xdg_runtime_dir = std::getenv("XDG_RUNTIME_DIR")) {
		const auto old_cwd = std::filesystem::current_path();
		std::filesystem::current_path(xdg_runtime_dir);
		for (size_t i = 0; i < 10; ++i) {
			std::string base = "discord-ipc-" + std::to_string(i);
			try {
				if (!std::filesystem::exists(base))
					std::filesystem::create_symlink("app/com.discordapp.Discord/" + base, base);
			} catch (const std::filesystem::filesystem_error &) { /* Necessary for some reason */ }
		}
		std::filesystem::current_path(old_cwd);
	}

	std::filesystem::current_path(".var/app/gay.heimskr.Game3/data");
	if (!std::filesystem::exists("resources"))
		std::filesystem::create_symlink(Game3::dataRoot / "resources", "resources");
#endif


	if (2 <= argc) {
		if (Game3::chemskrTest(argc, argv))
			return 0;

		if (strcmp(argv[1], "--token") == 0 && argc == 3) {
			if (!std::filesystem::exists(".secret")) {
				std::cerr << "Can't find .secret\n";
				return 1;
			}

			const std::string secret = Game3::readFile(".secret");
			std::cout << Game3::computeSHA3_512<Game3::Token>(secret + '/' + argv[2]) << '\n';
			return 0;
		}

		if (strcmp(argv[1], "--is-flatpak") == 0) {
#ifdef IS_FLATPAK
			std::cout << "Is Flatpak: \e[1;32mtrue\e[22;39m\n";
#else
			std::cout << "Is Flatpak: \e[1;31mfalse\e[22;39m\n";
#endif
			return 0;
		}

		if (strcmp(argv[1], "-s") == 0) {
			const auto out = Game3::Server::main(argc, argv);
			Game3::Timer::summary();
			return out;
		}

		if (strcmp(argv[1], "-t") == 0) {
			Game3::test();
			return 0;
		}

		if (strcmp(argv[1], "--split") == 0) {
			Game3::splitter();
			return 0;
		}

		if (strcmp(argv[1], "--tile-stitch") == 0) {
			if (argc == 3) {
				const auto count = Game3::parseNumber<size_t>(argv[2]);
				size_t dimension = 16;
				if (16 < count)
					dimension = 4 * size_t(std::pow(2, std::ceil(std::log2(std::ceil(std::sqrt(count))))));
				std::cout << count << " → " << dimension << 'x' << dimension << '\n';
				return 0;
			}

			std::string png;
			Game3::tileStitcher("resources/tileset", "base:tileset/monomap", &png);
			std::cout << png;
			return 0;
		}

		if (strcmp(argv[1], "--item-stitch") == 0) {
			std::string png;
			Game3::itemStitcher(nullptr, "resources/items", "base:itemset/items", &png);
			std::cout << png;
			return 0;
		}

		if (strcmp(argv[1], "--migrate") == 0) {
			std::vector<std::string> args;
			for (int i = 2; i < argc; ++i)
				args.emplace_back(argv[i]);
			return Game3::migrate(args);
		}

		if (strcmp(argv[1], "--maze") == 0) {
			for (const auto &row: Game3::Mazer({32, 32}, 666, {2, 0}).getRows(false)) {
				for (const auto column: row)
					std::cout << (column? "\u2588" : " ");
				std::cout << std::endl;
			}

			return 0;
		}

		if (strcmp(argv[1], "--omni-opt-out") == 0) {
			Game3::omniOptOut();
			return 0;
		}

		if (strcmp(argv[1], "--filter-test") == 0) {
			Game3::filterTest();
			return 0;
		}

		if (argc == 4) {
			std::cout << Game3::generateFlask(Game3::dataRoot / "resources" / "testtubebase.png", Game3::dataRoot / "resources" / "testtubemask.png", argv[1], argv[2], argv[3]);
			return 0;
		}
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	Game3::richPresence.init();
	Game3::richPresence.initActivity();

	auto app = Game3::App::create();
	const int out = app->run(argc, argv);
	Game3::Timer::summary();
	Game3::richPresence.reset();
	return out;
}
