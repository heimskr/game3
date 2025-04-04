#include "game/ClientGame.h"
#include "graphics/RealmRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "graphics/Tileset.h"
#include "realm/Realm.h"
#include "ui/GameUI.h"
#include "ui/Window.h"
#include "util/Util.h"

namespace Game3 {
	void GameUI::init(Window &) {
		Dialog::init();
		fbo.init();
	}

	void GameUI::render(const RendererContext &renderers) {
		Window &window = ui.window;

		const float x_factor = window.getXFactor();
		const float y_factor = window.getYFactor();
		auto [width, height] = window.getDimensions();

		ClientGamePtr game = window.game;

		if (!game) {
			window.rectangleRenderer.drawOnScreen(Color{"#ff0000"}, Rectangle(0, 0, width, height));
			return;
		}

		game->iterateRealms([](const RealmPtr &realm) {
			if (!realm->renderersReady) {
				return;
			}

			if (realm->wakeupPending.exchange(false)) {
				for (auto &row: *realm->baseRenderers) {
					for (auto &renderer: row) {
						renderer.wakeUp();
					}
				}

				for (auto &row: *realm->upperRenderers) {
					for (auto &renderer: row) {
						renderer.wakeUp();
					}
				}

				realm->reupload();
			} else if (realm->snoozePending.exchange(false)) {
				for (auto &row: *realm->baseRenderers) {
					for (auto &renderer: row) {
						renderer.snooze();
					}
				}

				for (auto &row: *realm->upperRenderers) {
					for (auto &renderer: row) {
						renderer.snooze();
					}
				}
			}
		});

		GLsizei tile_size = 16;
		RealmPtr realm = game->getActiveRealm();

		if (realm) {
			tile_size = static_cast<GLsizei>(realm->getTileset().getTileSize());
		}

		const auto x_static_size = static_cast<GLsizei>(REALM_DIAMETER * CHUNK_SIZE * tile_size * x_factor);
		const auto y_static_size = static_cast<GLsizei>(REALM_DIAMETER * CHUNK_SIZE * tile_size * y_factor);

		if (mainGLTexture.getWidth() != width || mainGLTexture.getHeight() != height) {
			mainGLTexture.initRGBA(width, height, GL_NEAREST);
			staticLightingTexture.initRGBA(x_static_size, y_static_size, GL_NEAREST);
			dynamicLightingTexture.initRGBA(width, height, GL_NEAREST);
			scratchGLTexture.initRGBA(width, height, GL_NEAREST);
			causticsGLTexture.initRGBA(width, height, GL_NEAREST);

			GL::FBOBinder binder = fbo.getBinder();
			dynamicLightingTexture.useInFB();
			GL::clear(1, 1, 1);

			if (realm) {
				realm->queueStaticLightingTexture();
			}

			mainTexture = std::make_shared<Texture>();
			mainTexture->init(mainGLTexture);

			scratchTexture = std::make_shared<Texture>();
			scratchTexture->init(scratchGLTexture);
		}

		if (realm) {
			realm->getRenderer()->render(window.getRendererContext(), realm, window, *this);
			realmBounds = game->getVisibleRealmBounds();
		}
	}

	Rectangle GameUI::getPosition() const {
		return ui.window.inset(0);
	}
}