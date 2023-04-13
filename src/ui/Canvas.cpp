#include <iostream>
#include <unordered_set>

#include "ui/Canvas.h"
#include "ui/MainWindow.h"

#include "game/Game.h"
#include "util/Timer.h"
#include "util/Util.h"

#include "Tileset.h"

namespace Game3 {
	Canvas::Canvas(MainWindow &window_): window(window_) {
		magic = 16 / 2;
		fbo.init();
	}

	void Canvas::drawGL() {
		if (!game)
			return;
		game->tick();
		spriteRenderer.update(width(), height());
		rectangleRenderer.update(width(), height());
		if (game->activeRealm)
			game->activeRealm->render(width(), height(), center, scale, spriteRenderer, game->getDivisor());
		realmBounds = game->getVisibleRealmBounds();
	}

	int Canvas::width() const {
		return window.glArea.get_width();
	}

	int Canvas::height() const {
		return window.glArea.get_height();
	}

	bool Canvas::inBounds(const Position &pos) const {
		return realmBounds.get_x() <= pos.column && pos.column < realmBounds.get_x() + realmBounds.get_width()
		    && realmBounds.get_y() <= pos.row    && pos.row    < realmBounds.get_y() + realmBounds.get_height();
	}
}
