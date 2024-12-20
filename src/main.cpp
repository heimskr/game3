#include "config.h"
#include "Log.h"
#include "Options.h"
#include "client/RichPresence.h"
#include "client/ServerWrapper.h"
#include "game/ClientGame.h"
#include "graphics/Texture.h"
#include "net/Server.h"
#include "scripting/ScriptEngine.h"
#include "tools/Flasker.h"
#include "tools/ItemStitcher.h"
#include "tools/Mazer.h"
#include "tools/Migrator.h"
#include "tools/TileStitcher.h"
#include "ui/gl/Constants.h"
#include "ui/Window.h"
#include "util/Crypto.h"
#include "util/Defer.h"
#include "util/FS.h"
#include "util/Shell.h"
#include "util/Timer.h"
#include "util/Util.h"

#define JC_VORONOI_IMPLEMENTATION
#include "jc_voronoi.h"

#include <GLFW/glfw3.h>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <vector>

namespace Game3 {
	void test();
	void testBuffer2();
	void omniOptOut();
	void filterTest();
	bool chemskrTest(int, char **);
	void skewTest(double location, double scale, double shape);
	void damageTest(HitPoints weapon_damage, int defense, int variability, double attacker_luck, double defender_luck);
	void voronoiTest();
	void scriptEngineTest();
	void zip8Test();
}

int main(int argc, char **argv) {
	using namespace Game3;

#ifdef GAME3_ENABLE_SCRIPTING
	ScriptEngine::init(argv[0]);
	Defer v8_deinit(ScriptEngine::deinit);
#endif

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
		std::filesystem::create_symlink(dataRoot / "resources", "resources");
#endif

	if (2 <= argc) {
		if (chemskrTest(argc, argv))
			return 0;

		const std::string_view arg1{argv[1]};

		if (arg1 == "--token" && argc == 3) {
			if (!std::filesystem::exists(".secret")) {
				std::cerr << "Can't find .secret\n";
				return 1;
			}

			const std::string secret = readFile(".secret");
			std::cout << computeSHA3_512<Token>(secret + '/' + argv[2]) << '\n';
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
			const auto out = Server::main(argc, argv);
			Timer::summary();
			return out;
		}

		if (arg1 == "-t") {
			test();
			return 0;
		}

		if (arg1 == "--skew" && argc == 5) {
			const double location = parseNumber<double>(argv[2]);
			const double scale = parseNumber<double>(argv[3]);
			const double shape = parseNumber<double>(argv[4]);
			skewTest(location, scale, shape);
			return 0;
		}

		if (arg1 == "--damage" && argc == 7) {
			const auto weapon_damage = parseNumber<HitPoints>(argv[2]);
			const auto defense = parseNumber<int>(argv[3]);
			const auto variability = parseNumber<int>(argv[4]);
			const auto attacker_luck = parseNumber<double>(argv[5]);
			const auto defender_luck = parseNumber<double>(argv[6]);
			damageTest(weapon_damage, defense, variability, attacker_luck, defender_luck);
			return 0;
		}

		if (arg1 == "--tile-stitch") {
			if (argc == 3) {
				const auto count = parseNumber<size_t>(argv[2]);
				size_t dimension = 16;
				if (16 < count)
					dimension = 4 * size_t(std::pow(2, std::ceil(std::log2(std::ceil(std::sqrt(count))))));
				std::cout << count << " → " << dimension << 'x' << dimension << '\n';
				return 0;
			}

			std::string png;
			tileStitcher("resources/tileset", "base:tileset/monomap", Side::Server, &png);
			std::cout << png;
			return 0;
		}

		if (arg1 == "--item-stitch") {
			std::string png;
			itemStitcher(nullptr, nullptr, "resources/items", "base:itemset/items", &png);
			std::cout << png;
			return 0;
		}

		if (arg1 == "--migrate") {
			std::vector<std::string> args;
			for (int i = 2; i < argc; ++i)
				args.emplace_back(argv[i]);
			return migrate(args);
		}

		if (arg1 == "--maze") {
			for (const auto &row: Mazer({32, 32}, 666, {2, 0}).getRows(false)) {
				for (const auto column: row)
					std::cout << (column? "\u2588" : " ");
				std::cout << std::endl;
			}

			return 0;
		}

		if (arg1 == "--omni-opt-out") {
			omniOptOut();
			return 0;
		}

		if (arg1 == "--filter-test") {
			filterTest();
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
				auto [out, err] = runCommand("./game3", {"--shell-test", "print"}, std::chrono::seconds(1), SIGINT);
				std::cout << std::format("stdout[{}], stderr[{}]\n", out, err);
			}

			{
				auto [out, err] = runCommand("./game3", {"--shell-test", "print"});
				std::cout << std::format("stdout[{}], stderr[{}]\n", out, err);
			}

			return 0;
		}

		if (arg1 == "--wrapper-test") {
			ServerWrapper wrapper;
			wrapper.run();
			return 0;
		}

		if (arg1 == "--voronoi-test") {
			voronoiTest();
			return 0;
		}

		if (arg1 == "--script-test") {
			scriptEngineTest();
			return 0;
		}

		if (arg1 == "--zip8-test") {
			zip8Test();
			return 0;
		}

		if (arg1 == "--buffer-test-2") {
			testBuffer2();
			return 0;
		}

		if (arg1 == "--ore-tile" && argc == 5) {
			std::cout << generateFlask(dataRoot / "resources" / "orebase.png", dataRoot / "resources" / "oremask.png", argv[2], argv[3], argv[4]);
			return 0;
		}

		if (arg1 == "--ore-item" && argc == 5) {
			std::cout << generateFlask(dataRoot / "resources" / "oreitembase.png", dataRoot / "resources" / "oreitemmask.png", argv[2], argv[3], argv[4]);
			return 0;
		}

		if (arg1 == "--erlen" && argc == 5) {
			std::cout << generateFlask(dataRoot / "resources" / "erlenmeyerbase.png", dataRoot / "resources" / "erlenmeyermask.png", argv[2], argv[3], argv[4]);
			return 0;
		}

		if (arg1 == "--testtube" && argc == 5) {
			std::cout << generateFlask(dataRoot / "resources" / "testtubebase.png", dataRoot / "resources" / "testtubemask.png", argv[2], argv[3], argv[4]);
			return 0;
		}

		if (arg1 == "--flask" && argc == 5) {
			std::cout << generateFlask(dataRoot / "resources" / "flaskbase.png", dataRoot / "resources" / "flaskmask.png", argv[2], argv[3], argv[4]);
			return 0;
		}
	}

	richPresence.init();
	richPresence.initActivity();

	if (!glfwInit()) {
		ERROR("Can't initialize GLFW");
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow *glfw_window = glfwCreateWindow(1600, 1000, "Game3", nullptr, nullptr);

	if (!glfw_window) {
		const char *message = "???";
		int code = glfwGetError(&message);
		ERROR("Can't create GLFW window: {} ({})", message, code);
		glfwTerminate();
		return 2;
	}

	glfwMaximizeWindow(glfw_window);
	glfwMakeContextCurrent(glfw_window);
	glfwSwapInterval(1);

	auto window = std::make_shared<Window>(*glfw_window);

	window->delay([](Window &window) {
		window.goToTitle();
	}, 4);

	SystemTimePoint time = getTime();

	constexpr std::array paths{
		"resources/gui/stone.png",
		"resources/gui/dirt.png",
		"resources/gui/grass.png",
		"resources/gui/grimrubble.png",
		"resources/tileset/lava/tile.png",
	};

	TexturePtr stone = cacheTexture(choose(paths, std::random_device{}));

	while (!glfwWindowShouldClose(glfw_window)) {
		GL::clear(0, 0, 0);
		if (!window->game) {
			constexpr float strength = 0.3;
			window->singleSpriteRenderer.drawOnScreen(stone, RenderOptions{
				.sizeX = static_cast<double>(window->getWidth()),
				.sizeY = static_cast<double>(window->getHeight()),
				.scaleX = 2 * UI_SCALE,
				.scaleY = 2 * UI_SCALE,
				.color{strength, strength, strength, 1},
				.invertY = false,
				.wrapMode = GL_REPEAT,
			});
		}
		window->tick();
		glfwSwapBuffers(glfw_window);
		glfwPollEvents();

		SystemTimePoint old_time = time;
		time = getTime();
		auto diff = std::chrono::duration_cast<std::chrono::microseconds>(time - old_time).count();
		if (diff != 0) {
			window->feedFPS(1e6 / diff);
		}

		if constexpr (SHOW_FPS_EVERY_FRAME) {
			if (diff == 0) {
				INFO("∞ FPS");
			} else {
				INFO("{} FPS", 1e6 / diff);
			}
		}
	}

	window->closeGame();
	window.reset();

	glfwTerminate();

	Timer::summary();
	richPresence.reset();
	return 0;
}
