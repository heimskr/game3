#include "game/ClientGame.h"
#include "graphics/RealmRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/Texture.h"
#include "graphics/Tileset.h"
#include "realm/Realm.h"
#include "ui/dialog/BottomDialog.h"
#include "ui/dialog/ChatDialog.h"
#include "ui/dialog/OmniDialog.h"
#include "ui/dialog/TopDialog.h"
#include "ui/module/FluidsModule.h"
#include "ui/module/InventoryModule.h"
#include "ui/module/ModuleFactory.h"
#include "ui/tab/InventoryTab.h"
#include "ui/GameUI.h"
#include "ui/Window.h"
#include "util/Util.h"

namespace Game3 {
	void GameUI::init(Window &) {
		Dialog::init();
		fbo.init();
		bottomDialog = ui.emplaceDialog<BottomDialog>(selfScale);
		chatDialog = ui.emplaceDialog<ChatDialog>(selfScale);
	}

	void GameUI::render(const RendererContext &renderers) {
		Window &window = ui.window;

		const float x_factor = renderers.xFactor;
		const float y_factor = renderers.yFactor;
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
			realm->getRenderer()->render(renderers, realm, window, *this);
			realmBounds = game->getVisibleRealmBounds();

			if (auto mouse = window.getMouseCoordinates<double>()) {
				const auto [mouse_x, mouse_y] = *mouse;
				Position hovered_position = game->translateCanvasCoordinates(mouse_x, mouse_y);
				TileEntityPtr previous_hovered = hoveredTileEntity.lock();
				if (TileEntityPtr tile_entity = realm->tileEntityAt(hovered_position)) {
					if (previous_hovered != tile_entity) {
						if (previous_hovered) {
							previous_hovered->mouseOut();
						}

						if (tile_entity->mouseOver()) {
							hoveredTileEntity = tile_entity;
						} else {
							hoveredTileEntity.reset();
						}
					}
				} else if (previous_hovered) {
					previous_hovered->mouseOut();
					hoveredTileEntity.reset();
				}
			}
		}
	}

	Rectangle GameUI::getPosition() const {
		return ui.window.inset(0);
	}

	const std::shared_ptr<OmniDialog> & GameUI::getOmniDialog() {
		if (!omniDialog) {
			omniDialog = make<OmniDialog>(ui, selfScale);
		}
		return omniDialog;
	}

	const std::shared_ptr<ChatDialog> & GameUI::getChatDialog() {
		return chatDialog;
	}

	const std::shared_ptr<TopDialog> & GameUI::getTopDialog() {
		if (!topDialog) {
			topDialog = make<TopDialog>(ui, selfScale);
		}
		return topDialog;
	}

	const std::shared_ptr<BottomDialog> & GameUI::getBottomDialog() {
		if (!bottomDialog) {
			bottomDialog = make<BottomDialog>(ui, selfScale);
		}
		return bottomDialog;
	}

	void GameUI::showOmniDialog() {
		if (!ui.hasDialog<OmniDialog>()) {
			ui.addDialog(getOmniDialog());
			ui.addDialog(getTopDialog());
		}
	}

	void GameUI::hideOmniDialog() {
		ui.removeDialogs<OmniDialog, TopDialog>();
	}

	void GameUI::toggleOmniDialog() {
		if (ui.hasDialog<OmniDialog>()) {
			hideOmniDialog();
		} else {
			showOmniDialog();
		}
	}

	void GameUI::openModule(const Identifier &module_id, const std::any &argument) {
		ClientGamePtr game = ui.window.game;
		assert(game != nullptr);

		auto &registry = game->registry<ModuleFactoryRegistry>();

		if (auto factory = registry[module_id]) {
			auto omni = getOmniDialog();
			omni->inventoryTab->setModule((*factory)(game, argument));
			omni->activeTab = omni->inventoryTab;
			showOmniDialog();
			return;
		}

		WARN("Couldn't find module {}", module_id);
	}

	void GameUI::removeModule() {
		if (omniDialog) {
			omniDialog->inventoryTab->removeModule();
		}
	}

	void GameUI::moduleMessageBuffer(const Identifier &module_id, const std::shared_ptr<Agent> &source, const std::string &name, Buffer &&buffer) {
		std::unique_lock<DefaultMutex> module_lock;

		if (Module *module_ = getOmniDialog()->inventoryTab->getModule(module_lock); module_ != nullptr && (module_id.empty() || module_->getID() == module_id)) {
			std::any data{std::move(buffer)};
			module_->handleMessage(source, name, data);
		}
	}

	void GameUI::showExternalInventory(const std::shared_ptr<ClientInventory> &inventory) {
		assert(inventory);
		getOmniDialog()->inventoryTab->setModule(std::make_shared<InventoryModule>(ui, 1, inventory));
	}

	void GameUI::showFluids(const std::shared_ptr<HasFluids> &has_fluids) {
		assert(has_fluids);
		getOmniDialog()->inventoryTab->setModule(std::make_shared<FluidsModule>(ui, 1, has_fluids));
	}

	GlobalID GameUI::getExternalGID() const {
		if (omniDialog) {
			std::unique_lock<DefaultMutex> lock;
			if (Module *module_ = omniDialog->inventoryTab->getModule(lock)) {
				std::any empty;
				if (std::optional<Buffer> response = module_->handleMessage({}, "GetAgentGID", empty)) {
					return response->take<GlobalID>();
				}
			}
		}

		return -1;
	}
}