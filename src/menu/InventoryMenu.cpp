#include <nanogui/opengl.h>
#include <nanogui/glutil.h>

#include <GL/glu.h>

#include "game/Game.h"
#include "menu/InventoryMenu.h"
#include "ui/Canvas.h"
#include "util/Util.h"

namespace Game3 {
	void InventoryMenu::render(Game &game, Canvas &canvas, NVGcontext *) {
		const auto width = canvas.width();
		const auto height = canvas.height();
		constexpr float x_fraction = 4.f;
		constexpr float y_fraction = 6.f;

		// glBegin(GL_POLYGON);
		// glColor4f(0.91f, 0.84f, 0.58f, 0.8f); CHECKGL
		// glRecti(width / x_fraction, height / y_fraction, width * (x_fraction - 1.f) / x_fraction, height * (y_fraction - 1.f) / y_fraction); CHECKGL

		// std::cout << glGetString(GL_VERSION) << '\n';

		canvas.rectangleRenderer({0.91f, 0.84f, 0.58f, 0.8f}, 200, 200, 100, 100);

		// glRecti(0, 0, 100, 100); CHECKGL

		// static Texture tex("resources/grass.png");
		// tex.init();
		// canvas.spriteRenderer.drawOnScreen(tex, 0, 0);


		// throw 42;
		// glVertex3f(width / x_fraction, height / y_fraction, 0.f);
		// glVertex3f(width * (x_fraction - 1.f) / x_fraction, height / y_fraction, 0.f);
		// glVertex3f(width * (x_fraction - 1.f) / x_fraction, height * (y_fraction - 1.f) / y_fraction, 0.f);
		// glVertex3f(width / x_fraction, height * (y_fraction - 1.f) / y_fraction, 0.f);
		// glEnd(); CHECKGL
	}	
}
