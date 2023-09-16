#include "config.h"
#include "App.h"
#include "Flasker.h"
#include "net/LocalServer.h"
#include "net/Sock.h"
#include "util/Crypto.h"
#include "util/FS.h"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <vector>

#include <GLFW/glfw3.h>

namespace Game3 {
	void test();
}

int main(int argc, char **argv) {
	srand(time(nullptr));

#ifdef IS_FLATPAK
	std::filesystem::current_path(".var/app/gay.heimskr.Game3/data");
	if (!std::filesystem::exists("resources"))
		std::filesystem::create_symlink(Game3::dataRoot / "resources", "resources");
#endif

	if (2 <= argc) {
		if (strcmp(argv[1], "--token") == 0 && argc == 3) {
			if (!std::filesystem::exists(".secret")) {
				std::cerr << "Can't find .secret\n";
				return 1;
			}

			std::string secret = Game3::readFile(".secret");
			std::cout << Game3::computeSHA3<Game3::Token>(secret + '/' + argv[2]) << '\n';
			return 0;
		}

		if (argc == 4) {
			std::cout << Game3::generateFlask(Game3::dataRoot / "resources" / "testtubebase.png", Game3::dataRoot / "resources" / "testtubemask.png", argv[1], argv[2], argv[3]);
			return 0;
		}

		if (strcmp(argv[1], "-s") == 0)
			return Game3::LocalServer::main(argc, argv);

		if (strcmp(argv[1], "-t") == 0) {
			Game3::test();
			return 0;
		}
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	auto app = Game3::App::create();
	const int out = app->run(argc, argv);
	return out;
}
