#include <cstdlib>
#include <ctime>
#include <iostream>

#include "ui/Application.h"

#include <GL/glut.h>
#include "Texture.h"
#include "ui/TilemapRenderer.h"

using namespace Game3;

Texture grass;
TilemapRenderer renderer;
std::shared_ptr<Tilemap> tilemap;

#define USE_NANOGUI

int main(int, char **) {
	srand(time(nullptr));
	try {
		nanogui::init();
		{
			nanogui::ref<Game3::Application> app = new Game3::Application;
			app->drawAll();
			app->setVisible(true);
			nanogui::mainloop(1000 / 144);
		}
		nanogui::shutdown();
		return 0;
	} catch (const std::exception &err) {
		std::cerr << err.what() << '\n';
		return 1;
	}
}
