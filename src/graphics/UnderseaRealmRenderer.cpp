#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "graphics/UnderseaRealmRenderer.h"
#include "graphics/RendererContext.h"
#include "realm/Realm.h"
#include "ui/Window.h"

namespace Game3 {
	namespace {
		constexpr Color SEA_COLOR{"#00a0d0"};
	}

	void UnderseaRealmRenderer::render(const RendererContext &renderers, const std::shared_ptr<Realm> &realm, Window &window) {
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
		}

		window.causticsShader.update(width, height);
		window.colorDodgeShader.update(width, height);

		ClientGamePtr game = window.game;

		realm->render(width, height, window.center, window.scale, renderers, game->getDivisor()); CHECKGL

		if (ClientPlayerPtr player = game->getPlayer()) {
			window.pathmapTextureCache.updateRealm(realm);
			window.pathmapTextureCache.visitChunk(player->getChunk());

			{
				auto lock = realm->pathmapUpdateSet.uniqueLock();
				auto set = std::move(realm->pathmapUpdateSet.getBase());
				lock.unlock();
				for (ChunkPosition chunk_position: set) {
					window.pathmapTextureCache.addChunk(chunk_position, true);
				}
			}

			window.causticsGLTexture.useInFB();
			GL::clear(1, 1, 1);

			ChunkRange(player->getChunk()).iterate([&](ChunkPosition visible_chunk) {
				if (TexturePtr pathmap = window.pathmapTextureCache.getTexture(visible_chunk)) {
					constexpr double size = CHUNK_SIZE * 16;
					const auto [row, column] = visible_chunk.topLeft();

					RenderOptions options{
						.x = static_cast<double>(column),
						.y = static_cast<double>(row),
						.sizeX = size,
						.sizeY = size,
					};

					window.causticsShader.shaderSetup = [&](Shader &shader, GLint) {
						pathmap->bind(2);
						shader.set("pathmap", 2);
						shader.set("time", static_cast<GLfloat>(game->time.load()));
						shader.set("mapCoord", static_cast<GLfloat>(column), static_cast<GLfloat>(row));
						shader.set("chunkSize", static_cast<GLfloat>(CHUNK_SIZE));
					};

					window.causticsShader.drawOnMap(size, size, options, realm->getTileset(), window);
				};
			});

			window.scratchGLTexture.useInFB();

			window.colorDodgeShader.shaderSetup = [&](Shader &shader, GLint) {
				window.causticsGLTexture.bind(2);
				shader.set("top", 2);
			};

			window.colorDodgeShader.drawOnScreen(window.mainGLTexture);
		}

		binder.undo();

		renderers.updateSize(width, height);
		glViewport(0, 0, width, height); CHECKGL

		if (window.settings.withShared([](const ClientSettings &settings) { return settings.specialEffects; })) {
			window.waveShader.shaderSetup = [&](Shader &shader, GLint) {
				shader.set("resolution", static_cast<GLfloat>(width), static_cast<GLfloat>(height));
				shader.set("time", static_cast<GLfloat>(game->time.load() * 0));
				shader.set("colorMultiplier", SEA_COLOR);
			};
			window.waveShader.update(width, height);
			window.waveShader.drawOnScreen(window.scratchTexture);
		} else {
			window.singleSpriteRenderer.drawOnScreen(window.scratchTexture, RenderOptions{
				.sizeX = -1,
				.sizeY = -1,
				.color = SEA_COLOR,
				.invertY = false,
			});
		}
	}
}
