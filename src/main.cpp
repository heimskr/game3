#include <cstdlib>
#include <ctime>
#include <iostream>

#include "ui/Application.h"

int main(int argc, char *argv[]) {
	srand(time(nullptr));
	try {
		nanogui::init();
		{
			nanogui::ref<Game3::Application> app = new Game3::Application;
			app->drawAll();
			app->setVisible(true);
			nanogui::mainloop();
		}
		nanogui::shutdown();
		return 0;
	} catch (const std::exception &err) {
		std::cerr << err.what() << '\n';
		return 1;
	}
}
