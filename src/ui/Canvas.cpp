#include <iostream>
#include <unordered_set>

#include "ui/Canvas.h"
#include "ui/MainWindow.h"

#include "game/ClientGame.h"
#include "util/Timer.h"
#include "util/Util.h"

#include "graphics/Tileset.h"

namespace Game3 {
	Canvas::Canvas(MainWindow &window_): window(window_) {
		magic = 16 / 2;
		fbo.init();
	}

	void Canvas::drawGL() {
		if (!game)
			return;
		game->activateContext();
		spriteRenderer.update(width(), height());
		rectangleRenderer.update(width(), height());
		textRenderer.update(width(), height());

		game->iterateRealms([](const RealmPtr &realm) {
			if (realm->wakeupPending.exchange(false)) {
				for (auto &row: *realm->renderers)
					for (auto &layers: row)
						for (auto &renderer: layers)
							renderer.wakeUp();

				for (auto &row: *realm->fluidRenderers)
					for (auto &renderer: row)
						renderer.wakeUp();

				realm->reupload();
			} else if (realm->snoozePending.exchange(false)) {
				for (auto &row: *realm->renderers)
					for (auto &layers: row)
						for (auto &renderer: layers)
							renderer.snooze();

				for (auto &row: *realm->fluidRenderers)
					for (auto &renderer: row)
						renderer.snooze();
			}
		});

		if (RealmPtr realm = game->activeRealm.copyBase()) {
			realm->render(width(), height(), center, scale, spriteRenderer, textRenderer, game->getDivisor());
			realmBounds = game->getVisibleRealmBounds();
		}
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
