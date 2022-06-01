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

int main(int argc, char *argv[]) {
	srand(time(nullptr));
	try {
#ifndef USE_NANOGUI
		constexpr int init_x = 640, init_y = 640;
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
		glutInitWindowPosition(2000, 1000);
		glutInitWindowSize(init_x, init_y);
		glutCreateWindow("What");
		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_DEBUG_OUTPUT);

		grass = Texture("resources/grass.png");
		grass.bind();
		int dimension = 640 / 32;
		int scale = 32;
		tilemap = std::make_shared<Tilemap>(dimension, dimension, scale, grass.id);
		int i = 0;
		for (int x = 0; x < dimension; ++x)
			for (int y = 0; y < dimension; ++y)
				// (*tilemap)(x, y) = ++i % 50;
				(*tilemap)(x, y) = rand() % 50;
		renderer.initialize(tilemap);
		renderer.onBackBufferResized(init_x, init_y);

		glutDisplayFunc(+[] {
			glClearColor(0.f, 0.2f, 0.f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT);
			grass.bind();
			std::cerr << __FILE__ << ':' << __LINE__ << ": !? " << glGetError() << '\n';
			renderer.render();
			glFlush();
		});
		glutReshapeFunc(+[](int w, int h) {
			renderer.center = {0.f, 0.f};
			renderer.onBackBufferResized(w, h);
		});
		glutMainLoop();
#else
		nanogui::init();
		{
			nanogui::ref<Game3::Application> app = new Game3::Application;
			app->drawAll();
			app->setVisible(true);
			nanogui::mainloop(1000 / 144);
		}
		nanogui::shutdown();
#endif
		return 0;
	} catch (const std::exception &err) {
		std::cerr << err.what() << '\n';
		return 1;
	}
}
