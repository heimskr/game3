#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>

#include "App.h"

namespace Game3 {
	void test();
}

int main(int argc, char **argv) {
	srand(time(nullptr));

	if (2 <= argc && strcmp(argv[1], "-t") == 0) {
		Game3::test();
		return 0;
	}

	auto app = Game3::App::create();
	const int out = app->run(argc, argv);
	return out;
}
