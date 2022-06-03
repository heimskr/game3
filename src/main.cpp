#include <cstdlib>
#include <ctime>
#include <iostream>

#include "ui/Application.h"

#include <GL/glut.h>
#include "Texture.h"
#include "ui/TilemapRenderer.h"

// #define CATCH

namespace Game3 {
	void test();
}

int main(int argc, char **argv) {
	srand(time(nullptr));

	if (2 <= argc && strcmp(argv[1], "-t") == 0) {
		Game3::test();
		return 0;
	}

#ifdef CATCH
	try {
#endif
		nanogui::init();
		{
			nanogui::ref<Game3::Application> app = new Game3::Application;
			app->drawAll();
			app->setVisible(true);
			nanogui::mainloop(1000 / 144);
		}
		nanogui::shutdown();
		return 0;
#ifdef CATCH
	} catch (const std::exception &err) {
		std::cerr << err.what() << '\n';
		return 1;
	}
#endif
}
