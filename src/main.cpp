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

// #define USE_NANOGUI

int main(int argc, char *argv[]) {
	srand(time(nullptr));
	try {
#ifndef USE_NANOGUI
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
		glutInitWindowPosition(2000, 1000);
		glutInitWindowSize(400, 300);
		glutCreateWindow("What");
		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_DEBUG_OUTPUT);

		grass = Texture("resources/grass.png");
		grass.bind();
		// renderer
		int dimension = 640 / 32;
		int scale = 32;
		tilemap = std::make_shared<Tilemap>(dimension * scale, dimension * scale, scale, grass.id);
		tilemap->tiles.resize(dimension * dimension);
		int i = 0;
		for (int x = 0; x < dimension; ++x)
			for (int y = 0; y < dimension; ++y)
				// (*tilemap)(x, y) = ++i % 50;
				(*tilemap)(x, y) = rand() % 50;
		renderer.initialize(tilemap);
		renderer.onBackBufferResized(200, 200);

		glutDisplayFunc(+[] {
			glClearColor(0.f, 0.2f, 0.f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT);
			grass.bind();
			std::cerr << __FILE__ << ':' << __LINE__ << ": !? " << glGetError() << '\n';
			renderer.render();
			glFlush();
		});
		glutMainLoop();
#else
		nanogui::init();
		{
			nanogui::ref<Game3::Application> app = new Game3::Application;
			app->drawAll();
			app->setVisible(true);
			nanogui::mainloop();
		}
		nanogui::shutdown();
#endif
		return 0;
	} catch (const std::exception &err) {
		std::cerr << err.what() << '\n';
		return 1;
	}
}
