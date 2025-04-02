#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "graphics/GL.h"
#include "graphics/RealmRenderer.h"
#include "graphics/RendererContext.h"
#include "realm/Realm.h"
#include "ui/Window.h"

namespace Game3 {
	static void renderWithLighting(const RendererContext &renderers, const std::shared_ptr<Realm> &realm, Window &window) {
		GL::FBOBinder binder = window.fbo.getBinder();
		window.mainGLTexture.useInFB();
		const auto [width, height] = window.getDimensions();
		glViewport(0, 0, width, height); CHECKGL
		GL::clear(.2, .2, .2);
		renderers.updateSize(width, height);

		if (realm->prerender()) {
			window.mainGLTexture.useInFB();
			window.batchSpriteRenderer.update(window);
			window.singleSpriteRenderer.update(window);
			window.recolor.update(window);
			window.textRenderer.update(window);
			renderers.updateSize(width, height);
			glViewport(0, 0, width, height); CHECKGL
			// Skip a frame to avoid glitchiness
			return;
		}

		ClientGamePtr game = window.game;

		realm->render(width, height, window.center, window.scale, renderers, game->getDivisor()); CHECKGL

		window.dynamicLightingTexture.useInFB();

		realm->renderLighting(width, height, window.center, window.scale, renderers, game->getDivisor()); CHECKGL

		window.scratchGLTexture.useInFB();
		GL::clear(1, 1, 1);

		window.singleSpriteRenderer.drawOnScreen(window.dynamicLightingTexture, RenderOptions{
			.x = 0,
			.y = 0,
			.sizeX = -1,
			.sizeY = -1,
			.scaleX = 1,
			.scaleY = 1,
		});

		ChunkPosition chunk = game->getPlayer()->getChunk() - ChunkPosition(1, 1);
		const auto [static_y, static_x] = chunk.topLeft();
		const auto x_factor = window.getXFactor();
		const auto y_factor = window.getYFactor();
		window.singleSpriteRenderer.drawOnMap(window.staticLightingTexture, RenderOptions{
			.x = static_cast<double>(static_x),
			.y = static_cast<double>(static_y),
			.sizeX = -1,
			.sizeY = -1,
			.scaleX = 1. / x_factor,
			.scaleY = 1. / y_factor,
			.viewportX = -static_cast<double>(x_factor),
			.viewportY = -static_cast<double>(y_factor),
		});

		binder.undo();

		renderers.updateSize(width, height);
		glViewport(0, 0, width, height); CHECKGL
		window.multiplier(window.mainGLTexture, window.scratchGLTexture);
	}

	static void renderWithoutLighting(const RendererContext &renderers, const std::shared_ptr<Realm> &realm, Window &window) {
		const auto [width, height] = window.getDimensions();
		glViewport(0, 0, width, height); CHECKGL
		GL::clear(.2, .2, .2);
		renderers.updateSize(width, height);

		if (realm->prerender()) {
			window.batchSpriteRenderer.update(window);
			window.singleSpriteRenderer.update(window);
			window.recolor.update(window);
			window.textRenderer.update(window);
			renderers.updateSize(width, height);
		}

		realm->render(width, height, window.center, window.scale, renderers, 1.f);
	}

	void RealmRenderer::render(const RendererContext &renderers, const std::shared_ptr<Realm> &realm, Window &window) {
		if (window.settings.renderLighting) { // don't care about data races on a boolean tbh
			renderWithLighting(renderers, realm, window);
		} else {
			renderWithoutLighting(renderers, realm, window);
		}
	}
}