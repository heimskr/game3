#include <iostream>
#include <unordered_set>

#include "ui/Canvas.h"
#include "ui/MainWindow.h"

#include "game/ClientGame.h"
#include "util/Timer.h"
#include "util/Util.h"

#include "graphics/BatchSpriteRenderer.h"
#include "graphics/RendererSet.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Tileset.h"

namespace Game3 {
	Canvas::Canvas(MainWindow &window_): window(window_) {
		magic = 16 / 2;
		fbo.init();
	}

	void Canvas::drawGL() {
		if (!game)
			return;

		const int factor = getFactor();
		int width  = getWidth() * factor;
		int height = getHeight() * factor;

		game->activateContext();
		batchSpriteRenderer.update(*this);
		singleSpriteRenderer.update(*this);
		rectangleRenderer.update(width, height);
		textRenderer.update(width, height);
		circleRenderer.update(width, height);
		multiplier.update(width, height);

		game->iterateRealms([](const RealmPtr &realm) {
			if (!realm->renderersReady)
				return;

			if (realm->wakeupPending.exchange(false)) {
				for (auto &row: *realm->baseRenderers)
					for (auto &renderer: row)
						renderer.wakeUp();

				for (auto &row: *realm->upperRenderers)
					for (auto &renderer: row)
						renderer.wakeUp();

				realm->reupload();
			} else if (realm->snoozePending.exchange(false)) {
				for (auto &row: *realm->baseRenderers)
					for (auto &renderer: row)
						renderer.snooze();

				for (auto &row: *realm->upperRenderers)
					for (auto &renderer: row)
						renderer.snooze();
			}
		});

		if (mainTexture.getWidth() != width || mainTexture.getHeight() != height) {
			mainTexture.initRGBA(width, height, GL_NEAREST);
			lightingTexture.initRGBA(width, height, GL_NEAREST);
		}

		if (RealmPtr realm = game->activeRealm.copyBase()) {
			fbo.bind();
			mainTexture.useInFB();
			GL::Viewport viewport(0, 0, width, height);
			batchSpriteRenderer.update(width, height);
			glClearColor(.2f, .2f, .2f, 1.f); CHECKGL
			glClear(GL_COLOR_BUFFER_BIT); CHECKGL
			RendererSet renderers{rectangleRenderer, batchSpriteRenderer, textRenderer, circleRenderer};
			realm->render(width, height, center, scale, renderers, game->getDivisor()); CHECKGL
			viewport.reset();
			lightingTexture.useInFB();
			realm->renderLighting(width, height, center, scale, renderers, game->getDivisor()); CHECKGL
			fbo.undo();
			multiplier(mainTexture, lightingTexture);
			realmBounds = game->getVisibleRealmBounds();
		}
	}

	int Canvas::getWidth() const {
		return window.glArea.get_width();
	}

	int Canvas::getHeight() const {
		return window.glArea.get_height();
	}

	int Canvas::getFactor() const {
		return 2;
	}

	bool Canvas::inBounds(const Position &pos) const {
		return realmBounds.get_x() <= pos.column && pos.column < realmBounds.get_x() + realmBounds.get_width()
		    && realmBounds.get_y() <= pos.row    && pos.row    < realmBounds.get_y() + realmBounds.get_height();
	}
}
