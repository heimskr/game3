#include <unordered_set>

#include "ui/Canvas.h"
#include "ui/MainWindow.h"

#include "game/Game.h"
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
				game->activeRealm->render(width(), height(), center, scale, spriteRenderer, std::chrono::duration_cast<std::chrono::nanoseconds>(getTime() - game->startTime).count() / 1e9f);
		}
	}

	int Canvas::width() {
		return window.glArea.get_width();
	}

	int Canvas::height() {
		return window.glArea.get_height();
	}
}
