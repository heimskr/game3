#include "App.h"

int main(int argc, char *argv[]) {
	srand(time(nullptr));
	auto app = Game3::App::create();
	return app->run(argc, argv);
}
