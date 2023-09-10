#include <iostream>
#include <unordered_set>

#include "ui/Canvas.h"
#include "ui/Nanogui.h"

#include "game/ClientGame.h"
#include "util/Timer.h"
#include "util/Util.h"

#include "Tileset.h"

namespace Game3 {
	Canvas::Canvas(Application &app_): nanogui::Canvas(&app_), app(app_) {
		magic = 16 / 2;
		fbo.init();
	}

	void Canvas::drawGL() {
		if (!game)
			return;

		// game->activateContext();
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

	// int Canvas::width() const {
	// 	return app.window()->width();
	// }

	// int Canvas::height() const {
	// 	return app.window()->height();
	// }

	bool Canvas::inBounds(const Position &pos) const {
		return realmBounds.get_x() <= pos.column && pos.column < realmBounds.get_x() + realmBounds.get_width()
		    && realmBounds.get_y() <= pos.row    && pos.row    < realmBounds.get_y() + realmBounds.get_height();
	}
}
