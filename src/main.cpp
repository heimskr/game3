#include "config.h"
#include "Log.h"
#include "client/RichPresence.h"
#include "client/ServerWrapper.h"
#include "net/Server.h"
#include "net/Sock.h"
#include "tools/Flasker.h"
#include "tools/ItemStitcher.h"
#include "tools/Mazer.h"
#include "tools/Migrator.h"
#include "tools/TileStitcher.h"
#include "ui/App.h"
#include "util/Crypto.h"
#include "util/FS.h"
#include "util/Shell.h"
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
	void skewTest(double location, double scale, double shape);
	void damageTest(HitPoints weapon_damage, int defense, int variability, double attacker_luck, double defender_luck);
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

		const std::string_view arg1{argv[1]};

		if (arg1 == "--token" && argc == 3) {
			if (!std::filesystem::exists(".secret")) {
				std::cerr << "Can't find .secret\n";
				return 1;
			}

			const std::string secret = Game3::readFile(".secret");
			std::cout << Game3::computeSHA3_512<Game3::Token>(secret + '/' + argv[2]) << '\n';
			return 0;
		}

		if (arg1 == "--is-flatpak") {
#ifdef IS_FLATPAK
			std::cout << "Is Flatpak: \e[1;32mtrue\e[22;39m\n";
#else
			std::cout << "Is Flatpak: \e[1;31mfalse\e[22;39m\n";
#endif
			return 0;
		}

		if (arg1 == "-s") {
			const auto out = Game3::Server::main(argc, argv);
			Game3::Timer::summary();
			return out;
		}

		if (arg1 == "-t") {
			Game3::test();
			return 0;
		}

		if (arg1 == "--skew" && argc == 5) {
			const double location = Game3::parseNumber<double>(argv[2]);
			const double scale = Game3::parseNumber<double>(argv[3]);
			const double shape = Game3::parseNumber<double>(argv[4]);
			Game3::skewTest(location, scale, shape);
			return 0;
		}

		if (arg1 == "--damage" && argc == 7) {
			const auto weapon_damage = Game3::parseNumber<Game3::HitPoints>(argv[2]);
			const auto defense = Game3::parseNumber<int>(argv[3]);
			const auto variability = Game3::parseNumber<int>(argv[4]);
			const auto attacker_luck = Game3::parseNumber<double>(argv[5]);
			const auto defender_luck = Game3::parseNumber<double>(argv[6]);
			Game3::damageTest(weapon_damage, defense, variability, attacker_luck, defender_luck);
			return 0;
		}

		if (arg1 == "--split") {
			Game3::splitter();
			return 0;
		}

		if (arg1 == "--tile-stitch") {
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

		if (arg1 == "--item-stitch") {
			std::string png;
			Game3::itemStitcher(nullptr, nullptr, "resources/items", "base:itemset/items", &png);
			std::cout << png;
			return 0;
		}

		if (arg1 == "--migrate") {
			std::vector<std::string> args;
			for (int i = 2; i < argc; ++i)
				args.emplace_back(argv[i]);
			return Game3::migrate(args);
		}

		if (arg1 == "--maze") {
			for (const auto &row: Game3::Mazer({32, 32}, 666, {2, 0}).getRows(false)) {
				for (const auto column: row)
					std::cout << (column? "\u2588" : " ");
				std::cout << std::endl;
			}

			return 0;
		}

		if (arg1 == "--omni-opt-out") {
			Game3::omniOptOut();
			return 0;
		}

		if (arg1 == "--filter-test") {
			Game3::filterTest();
			return 0;
		}

		if (arg1 == "--shell-test") {
			if (argc == 3 && strcmp(argv[2], "print") == 0) {
				std::cout << "Hello, ";
				std::cerr << "Here is ";
				std::cout << "World!";
				std::cerr << "some text.";
				std::this_thread::sleep_for(std::chrono::seconds(3));
				std::cout << " Here's more text after three seconds.";
				return 0;
			}

			{
				auto [out, err] = Game3::runCommand("./game3", {"--shell-test", "print"}, std::chrono::seconds(1), SIGINT);
				std::cout << std::format("stdout[{}], stderr[{}]\n", out, err);
			}

			{
				auto [out, err] = Game3::runCommand("./game3", {"--shell-test", "print"});
				std::cout << std::format("stdout[{}], stderr[{}]\n", out, err);
			}

			return 0;
		}

		if (arg1 == "--wrapper-test") {
			Game3::ServerWrapper wrapper;
			wrapper.run();
			return 0;
		}

		if (argc == 4) {
			// std::cout << Game3::generateFlask(Game3::dataRoot / "resources" / "erlenmeyerbase.png", Game3::dataRoot / "resources" / "erlenmeyermask.png", argv[1], argv[2], argv[3]);
			// std::cout << Game3::generateFlask(Game3::dataRoot / "resources" / "testtubebase.png", Game3::dataRoot / "resources" / "testtubemask.png", argv[1], argv[2], argv[3]);
			std::cout << Game3::generateFlask(Game3::dataRoot / "resources" / "flaskbase.png", Game3::dataRoot / "resources" / "flaskmask.png", argv[1], argv[2], argv[3]);
			return 0;
		}
	}

	Game3::richPresence.init();
	Game3::richPresence.initActivity();

	auto app = Game3::App::create();
	const int out = app->run(argc, argv);
	Game3::Timer::summary();
	Game3::richPresence.reset();
	return out;
}
