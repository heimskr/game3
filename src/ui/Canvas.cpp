#include <iostream>
#include <unordered_set>

#include "ui/Canvas.h"
#include "ui/MainWindow.h"

#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "util/Timer.h"
#include "util/Util.h"

#include "graphics/BatchSpriteRenderer.h"
#include "graphics/RendererContext.h"
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
		overlayer.update(width, height);

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

		GLsizei tile_size = 16;
		RealmPtr realm = game->activeRealm.copyBase();

		if (realm)
			tile_size = static_cast<GLsizei>(realm->getTileset().getTileSize());

		const auto static_size = static_cast<GLsizei>(REALM_DIAMETER * CHUNK_SIZE * tile_size * factor);

		if (mainTexture.getWidth() != width || mainTexture.getHeight() != height) {
			mainTexture.initRGBA(width, height, GL_NEAREST);
			staticLightingTexture.initRGBA(static_size, static_size, GL_NEAREST);
			dynamicLightingTexture.initRGBA(width, height, GL_NEAREST);
			scratchTexture.initRGBA(width, height, GL_NEAREST);
			fbo.bind();
			dynamicLightingTexture.useInFB();
			GL::clear(1, 1, 1);
			fbo.undo();
		}

		if (RealmPtr realm = game->activeRealm.copyBase()) {
			fbo.bind();

			mainTexture.useInFB();
			GL::Viewport viewport(0, 0, width, height);
			GL::clear(.2, .2, .2);
			RendererContext context = getRendererContext();
			context.updateSize(width, height);
			realm->render(width, height, center, scale, context, game->getDivisor()); CHECKGL

			dynamicLightingTexture.useInFB();
			realm->renderLighting(width, height, center, scale, context, game->getDivisor()); CHECKGL

			scratchTexture.useInFB();
			GL::clear(1, 1, 1, 1);

			singleSpriteRenderer.drawOnScreen(dynamicLightingTexture, RenderOptions{
				.x = 0.0,
				.y = 0.0,
				.sizeX = -1.0,
				.sizeY = -1.0,
				.scaleX = 1.,
				.scaleY = 1.,
			});

			ChunkPosition chunk = game->player->getChunk() - ChunkPosition(1, 1);
			const auto [static_y, static_x] = chunk.topLeft();
			singleSpriteRenderer.drawOnMap(staticLightingTexture, RenderOptions{
				.x = double(static_x),
				.y = double(static_y),
				.sizeX = -1.0,
				.sizeY = -1.0,
				.scaleX = 1. / factor,
				.scaleY = 1. / factor,
				.viewportX = -double(factor),
				.viewportY = -double(factor),
			});

			viewport.reset();
			fbo.undo();

			context.updateSize(getWidth(), getHeight());
			multiplier(mainTexture, scratchTexture);
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

	RendererContext Canvas::getRendererContext() {
		return {rectangleRenderer, singleSpriteRenderer, batchSpriteRenderer, textRenderer, circleRenderer, getFactor()};
	}
}
