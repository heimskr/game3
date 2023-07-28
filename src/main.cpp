#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <vector>

#include "App.h"
#include "Flasker.h"
#include "net/LocalServer.h"

namespace Game3 {
	void test();
}

#include "net/Sock.h"

int main(int argc, char **argv) {
	srand(time(nullptr));

	if (2 <= argc) {
		if (argc == 4) {
			std::cout << Game3::generateFlask("resources/testtubebase.png", "resources/testtubemask.png", argv[1], argv[2], argv[3]);
			return 0;
		}

		if (strcmp(argv[1], "-s") == 0)
			return Game3::LocalServer::main(argc, argv);

		if (strcmp(argv[1], "-t") == 0) {
			Game3::test();
			return 0;
		}
	}

	auto app = Game3::App::create();
	const int out = app->run(argc, argv);
	return out;
}
