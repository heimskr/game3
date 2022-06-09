#include <unordered_set>

#include <libnoise/noise.h>

#include "ui/Canvas.h"
#include "ui/MainWindow.h"

#include "game/Game.h"
// #include "menu/Menu.h"
#include "util/Timer.h"
#include "util/Util.h"

#include "Tiles.h"

namespace Game3 {
	Canvas::Canvas(MainWindow &window_): window(window_) {
		magic = 16 / 2;
	}

	void Canvas::drawGL() {
		if (game) {
			game->tick();
			spriteRenderer.update(width(), height());
			rectangleRenderer.update(width(), height());
			if (game->activeRealm)
				game->activeRealm->render(width(), height(), center, scale, spriteRenderer);
		}
	}

	int Canvas::width() {
		return window.glArea.get_width();
	}

	int Canvas::height() {
		return window.glArea.get_height();
	}
}
