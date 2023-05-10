#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>

#include "App.h"
#include "net/GameServer.h"

namespace Game3 {
	void test();
}

int main(int argc, char **argv) {
	srand(time(nullptr));

	if (2 <= argc) {
		if (strcmp(argv[1], "-s") == 0)
			return Game3::GameServer::main(argc, argv);

		if (strcmp(argv[1], "-t") == 0) {
			Game3::test();
			return 0;
		}
	}

	auto app = Game3::App::create();
	const int out = app->run(argc, argv);
	return out;
}
