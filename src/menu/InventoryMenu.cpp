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

		float pad = 10.f;

		const float x = width / x_fraction;
		const float y = height / y_fraction;
		const float w = width * (x_fraction - 2.f) / x_fraction;
		const float h = height * (y_fraction - 2.f) / y_fraction;

		canvas.rectangleRenderer({0.81f, 0.74f, 0.48f, 1.f}, x - pad, y - pad, w + 2 * pad, h + 2 * pad);
		canvas.rectangleRenderer({0.91f, 0.84f, 0.58f, 1.f}, x, y, w, h);
	}	
}
