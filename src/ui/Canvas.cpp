#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "graphics/BatchSpriteRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Tileset.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "util/Timer.h"
#include "util/Util.h"

#include "lib/stb/stb_image.h"
#include "lib/stb/stb_image_write.h"

namespace Game3 {
	Canvas::Canvas(MainWindow &window_): window(window_) {
		magic = 16 / 2;
		fbo.init();
	}

	void Canvas::drawGL() {
		if (!game)
			return;

		const int factor = getFactor();
		int width  = getWidth()  * factor;
		int height = getHeight() * factor;

		game->activateContext();
		batchSpriteRenderer.update(*this);
		singleSpriteRenderer.update(*this);
		textRenderer.update(*this);
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
		RealmPtr realm = game->getActiveRealm();

		if (realm)
			tile_size = static_cast<GLsizei>(realm->getTileset().getTileSize());

		const auto static_size = static_cast<GLsizei>(REALM_DIAMETER * CHUNK_SIZE * tile_size * factor);

		if (mainTexture.getWidth() != width || mainTexture.getHeight() != height) {
			mainTexture.initRGBA(width, height, GL_NEAREST);
			staticLightingTexture.initRGBA(static_size, static_size, GL_NEAREST);
			dynamicLightingTexture.initRGBA(width, height, GL_NEAREST);
			scratchTexture.initRGBA(width, height, GL_NEAREST);

			GL::FBOBinder binder = fbo.getBinder();
			dynamicLightingTexture.useInFB();
			GL::clear(1, 1, 1);

			if (realm) {
				realm->queueStaticLightingTexture();
			}
		}

		bool do_lighting{};
		{
			auto lock = window.settings.sharedLock();
			do_lighting = window.settings.renderLighting;
		}

		if (!realm)
			return;

		if (do_lighting) {
			GL::FBOBinder binder = fbo.getBinder();
			mainTexture.useInFB();
			glViewport(0, 0, width, height); CHECKGL
			GL::clear(.2, .2, .2);
			RendererContext context = getRendererContext();
			context.updateSize(width, height);

			if (realm->prerender()) {
				mainTexture.useInFB();
				batchSpriteRenderer.update(*this);
				singleSpriteRenderer.update(*this);
				textRenderer.update(*this);
				context.updateSize(getWidth(), getHeight());
				glViewport(0, 0, width, height); CHECKGL
				// Skip a frame to avoid glitchiness
				return;
			}

			realm->render(width, height, center, scale, context, game->getDivisor()); CHECKGL

			dynamicLightingTexture.useInFB();

			realm->renderLighting(width, height, center, scale, context, game->getDivisor()); CHECKGL

			scratchTexture.useInFB();
			GL::clear(1, 1, 1);

			singleSpriteRenderer.drawOnScreen(dynamicLightingTexture, RenderOptions{
				.x = 0,
				.y = 0,
				.sizeX = -1,
				.sizeY = -1,
				.scaleX = 1,
				.scaleY = 1,
			});

			ChunkPosition chunk = game->getPlayer()->getChunk() - ChunkPosition(1, 1);
			const auto [static_y, static_x] = chunk.topLeft();
			singleSpriteRenderer.drawOnMap(staticLightingTexture, RenderOptions{
				.x = double(static_x),
				.y = double(static_y),
				.sizeX = -1,
				.sizeY = -1,
				.scaleX = 1. / factor,
				.scaleY = 1. / factor,
				.viewportX = -double(factor),
				.viewportY = -double(factor),
			});

			binder.undo();

			context.updateSize(getWidth(), getHeight());
			glViewport(0, 0, width, height); CHECKGL
			multiplier(mainTexture, scratchTexture);

		} else {
			RendererContext context = getRendererContext();
			glViewport(0, 0, width, height); CHECKGL
			GL::clear(.2, .2, .2);
			context.updateSize(width, height);

			if (realm->prerender()) {
				batchSpriteRenderer.update(*this);
				singleSpriteRenderer.update(*this);
				textRenderer.update(*this);
				context.updateSize(width, height);
			}

			realm->render(width, height, center, scale, context, 1.f); CHECKGL
		}

		realmBounds = game->getVisibleRealmBounds();
	}

	int Canvas::getWidth() const {
		return window.glArea.get_width() / sizeDivisor;
	}

	int Canvas::getHeight() const {
		return window.glArea.get_height() / sizeDivisor;
	}

	int Canvas::getFactor() const {
		return 2;
	}

	bool Canvas::inBounds(const Position &pos) const {
		const auto x = realmBounds.get_x();
		const auto y = realmBounds.get_y();
		return x <= pos.column && pos.column < x + realmBounds.get_width()
		    && y <= pos.row    && pos.row    < y + realmBounds.get_height();
	}

	RendererContext Canvas::getRendererContext() {
		return {rectangleRenderer, singleSpriteRenderer, batchSpriteRenderer, textRenderer, circleRenderer, window.settings, getFactor()};
	}
}
