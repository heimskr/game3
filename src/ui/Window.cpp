#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "graphics/RendererContext.h"
#include "graphics/Tileset.h"
#include "net/LocalClient.h"
#include "packet/ContinuousInteractionPacket.h"
#include "packet/LoginPacket.h"
#include "packet/SetHeldItemPacket.h"
#include "types/Position.h"
#include "ui/gl/module/FluidsModule.h"
#include "ui/gl/module/InventoryModule.h"
#include "ui/gl/module/ModuleFactory.h"
#include "ui/gl/tab/InventoryTab.h"
#include "ui/gl/dialog/DraggableDialog.h"
#include "ui/gl/dialog/OmniDialog.h"
#include "ui/Window.h"
#include "ui/Modifiers.h"
#include "ui/Window.h"
#include "util/FS.h"
#include "util/Util.h"

#include <fstream>

namespace Game3 {
	namespace {
		constexpr std::chrono::milliseconds KEY_REPEAT_TIME{100};
		constexpr std::chrono::milliseconds ARROW_TIME{100};
		constexpr std::chrono::milliseconds INTERACT_TIME{250};
		constexpr std::chrono::milliseconds JUMP_TIME{50};
		constexpr std::chrono::milliseconds SLOW_TIME{1'000};
		constexpr std::chrono::milliseconds FOREVER{1'000'000'000};

		std::map<guint, std::chrono::milliseconds> CUSTOM_KEY_REPEAT_TIMES{
			{GDK_KEY_Up,           ARROW_TIME},
			{GDK_KEY_Down,         ARROW_TIME},
			{GDK_KEY_Left,         ARROW_TIME},
			{GDK_KEY_Right,        ARROW_TIME},
			{GDK_KEY_q,            INTERACT_TIME},
			{GDK_KEY_Q,            INTERACT_TIME},
			{GDK_KEY_e,            INTERACT_TIME},
			{GDK_KEY_E,            INTERACT_TIME},
			{GDK_KEY_bracketleft,  INTERACT_TIME},
			{GDK_KEY_bracketright, INTERACT_TIME},
			{GDK_KEY_r,            INTERACT_TIME},
			{GDK_KEY_R,            INTERACT_TIME},
			{GDK_KEY_o,            INTERACT_TIME},
			{GDK_KEY_Return,       INTERACT_TIME},
			{GDK_KEY_space,        JUMP_TIME},
			{GDK_KEY_g,            SLOW_TIME},
			{GDK_KEY_0,            SLOW_TIME},
			{GDK_KEY_1,            SLOW_TIME},
			{GDK_KEY_2,            SLOW_TIME},
			{GDK_KEY_3,            SLOW_TIME},
			{GDK_KEY_4,            SLOW_TIME},
			{GDK_KEY_5,            SLOW_TIME},
			{GDK_KEY_6,            SLOW_TIME},
			{GDK_KEY_7,            SLOW_TIME},
			{GDK_KEY_8,            SLOW_TIME},
			{GDK_KEY_9,            SLOW_TIME},
			{GDK_KEY_braceleft,    SLOW_TIME},
			{GDK_KEY_braceright,   SLOW_TIME},
			{GDK_KEY_Escape,       FOREVER},
		};
	}

	Window::Window(GLFWwindow &glfw_window):
		glfwWindow(&glfw_window),
		scale(8) {
			glfwSetWindowUserPointer(glfwWindow, this);

			glfwSetKeyCallback(glfwWindow, +[](GLFWwindow *glfw_window, int key, int scancode, int action, int mods) {
				reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw_window))->keyCallback(key, scancode, action, mods);
			});

			glfwSetMouseButtonCallback(glfwWindow, +[](GLFWwindow *glfw_window, int button, int action, int mods) {
				reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw_window))->mouseButtonCallback(button, action, mods);
			});

			glfwSetCursorPosCallback(glfwWindow, +[](GLFWwindow *glfw_window, double x_pos, double y_pos) {
				const auto x = static_cast<int>(std::floor(x_pos));
				const auto y = static_cast<int>(std::floor(y_pos));
				reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw_window))->mousePositionCallback(x, y);
			});

			glfwSetScrollCallback(glfwWindow, +[](GLFWwindow *glfw_window, double x, double y) {
				reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw_window))->scrollCallback(x, y);
			});

			if (std::filesystem::exists("settings.json"))
				settings = nlohmann::json::parse(readFile("settings.json"));

			settings.apply();
		}

	void Window::queue(std::function<void(Window &)> function) {
		functionQueue.push(std::move(function));
	}

	void Window::queueBool(std::function<bool(Window &)> function) {
		auto lock = boolFunctions.uniqueLock();
		boolFunctions.push_back(std::move(function));
	}

	void Window::delay(std::function<void(Window &)> function, uint32_t count) {
		if (count <= 0) {
			queue(std::move(function));
		} else {
			queue([function = std::move(function), count](Window &window) mutable {
				window.delay(std::move(function), count - 1);
			});
		}
	}

	int Window::getWidth() const {
		int width;
		glfwGetWindowSize(glfwWindow, &width, nullptr);
		return width;
	}

	int Window::getHeight() const {
		int height;
		glfwGetWindowSize(glfwWindow, nullptr, &height);
		return height;
	}

	int Window::getFactor() const {
		return 1;
	}

	int Window::getMouseX() const {
		double x{};
		glfwGetCursorPos(glfwWindow, &x, nullptr);
		return static_cast<int>(std::floor(x));
	}

	int Window::getMouseY() const {
		double y{};
		glfwGetCursorPos(glfwWindow, nullptr, &y);
		return static_cast<int>(std::floor(y));
	}

	std::pair<double, double> Window::getMouseCoordinates() const {
		double x{}, y{};
		glfwGetCursorPos(glfwWindow, &x, &y);
		return {x, y};
	}

	const std::shared_ptr<OmniDialog> & Window::getOmniDialog() {
		if (!omniDialog)
			omniDialog = std::make_shared<OmniDialog>(uiContext);
		return omniDialog;
	}

	void Window::showOmniDialog() {
		if (!uiContext.hasDialog<OmniDialog>())
			uiContext.addDialog(getOmniDialog());
	}

	void Window::closeOmniDialog() {
		uiContext.removeDialogs<OmniDialog>();
	}

	void Window::openModule(const Identifier &module_id, const std::any &argument) {
		assert(game != nullptr);

		auto &registry = game->registry<ModuleFactoryRegistry>();

		if (auto factory = registry[module_id]) {
			getOmniDialog();
			omniDialog->inventoryTab->setModule((*factory)(game, argument));
			omniDialog->activeTab = omniDialog->inventoryTab;
			if (!uiContext.hasDialog<OmniDialog>())
				uiContext.addDialog(omniDialog);
			return;
		}

		WARN("Couldn't find module {}", module_id);
	}

	void Window::removeModule() {
		if (omniDialog) {
			omniDialog->inventoryTab->removeModule();
		}
	}

	void Window::alert(const UString &message, bool queue, bool modal, bool use_markup) {
		(void) queue; (void) modal; (void) use_markup;

		auto dialog = std::make_shared<DraggableDialog>(uiContext, 600, 400);
		dialog->init();
		dialog->setTitle(message);
		uiContext.addDialog(std::move(dialog));

		INFO("Alert: {}", message);
	}

	void Window::error(const UString &message, bool queue, bool modal, bool use_markup) {
		(void) queue; (void) modal; (void) use_markup;
		ERROR("Error: {}", message);
	}

	Modifiers Window::getModifiers() const {
		return lastModifiers;
	}

	Position Window::getHoveredPosition() const {
		return {};
	}

	void Window::moduleMessageBuffer(const Identifier &module_id, const std::shared_ptr<Agent> &source, const std::string &name, Buffer &&buffer) {
		std::unique_lock<DefaultMutex> module_lock;

		if (Module *module_ = getOmniDialog()->inventoryTab->getModule(module_lock); module_ != nullptr && (module_id.empty() || module_->getID() == module_id)) {
			std::any data{std::move(buffer)};
			module_->handleMessage(source, name, data);
		}
	}

	void Window::activateContext() {
		glfwMakeContextCurrent(glfwWindow);
	}

	void Window::saveSettings() {
		std::ofstream ofs("settings.json");
		nlohmann::json json;
		{
			auto lock = settings.sharedLock();
			json = settings;
		}
		ofs << json.dump();
	}

	void Window::showExternalInventory(const std::shared_ptr<ClientInventory> &inventory) {
		assert(inventory);
		getOmniDialog()->inventoryTab->setModule(std::make_shared<InventoryModule>(uiContext, inventory));
	}

	void Window::showFluids(const std::shared_ptr<HasFluids> &has_fluids) {
		assert(has_fluids);
		getOmniDialog()->inventoryTab->setModule(std::make_shared<FluidsModule>(uiContext, has_fluids));
	}

	GlobalID Window::getExternalGID() const {
		if (omniDialog) {
			std::unique_lock<DefaultMutex> lock;
			if (Module *module_ = omniDialog->inventoryTab->getModule(lock)) {
				std::any empty;
				if (std::optional<Buffer> response = module_->handleMessage({}, "GetAgentGID", empty))
					return response->take<GlobalID>();
			}
		}

		return -1;
	}

	bool Window::inBounds(const Position &pos) const {
		const auto x = realmBounds.x;
		const auto y = realmBounds.y;
		return x <= pos.column && pos.column < x + realmBounds.width
		    && y <= pos.row    && pos.row    < y + realmBounds.height;
	}

	RendererContext Window::getRendererContext() {
		return {rectangleRenderer, singleSpriteRenderer, batchSpriteRenderer, textRenderer, circleRenderer, recolor, settings, getFactor()};
	}

	void Window::tick() {
		int width{}, height{};
		glfwGetWindowSize(glfwWindow, &width, &height);
		if (width != lastWindowSize.first || height != lastWindowSize.second) {
			uiContext.onResize(width, height);
			lastWindowSize = {width, height};
		}

		handleKeys();

		for (const auto &function: functionQueue.steal())
			function(*this);

		if (autofocus && game) {
			if (ClientPlayerPtr player = game->getPlayer()) {
				player->focus(*this, true);
			}
		}

		{
			auto lock = boolFunctions.uniqueLock();
			std::erase_if(boolFunctions, [this](const auto &function) {
				return function(*this);
			});
		}

		drawGL();
	}

	void Window::drawGL() {
		const int factor = getFactor();
		int width  = getWidth()  * factor;
		int height = getHeight() * factor;

		activateContext();
		batchSpriteRenderer.update(*this);
		singleSpriteRenderer.update(*this);
		recolor.update(*this);
		textRenderer.update(*this);
		rectangleRenderer.update(width, height);
		textRenderer.update(width, height);
		circleRenderer.update(width, height);
		multiplier.update(width, height);
		overlayer.update(width, height);

		if (game != nullptr) {
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
				auto lock = settings.sharedLock();
				do_lighting = settings.renderLighting;
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
					recolor.update(*this);
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
					.x = static_cast<double>(static_x),
					.y = static_cast<double>(static_y),
					.sizeX = -1,
					.sizeY = -1,
					.scaleX = 1. / factor,
					.scaleY = 1. / factor,
					.viewportX = -static_cast<double>(factor),
					.viewportY = -static_cast<double>(factor),
				});

				binder.undo();

				context.updateSize(width, height);
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
					recolor.update(*this);
					textRenderer.update(*this);
					context.updateSize(width, height);
				}

				realm->render(width, height, center, scale, context, 1.f); CHECKGL
			}

			realmBounds = game->getVisibleRealmBounds();
		}

		uiContext.render(getMouseX(), getMouseY());
	}

	void Window::keyCallback(int key, int scancode, int action, int raw_modifiers) {
		const Modifiers modifiers(static_cast<uint8_t>(raw_modifiers));

		lastModifiers = modifiers;

		if (action == GLFW_PRESS) {
			if (auto iter = keyTimes.find(key); iter != keyTimes.end()) {
				iter->second.modifiers = modifiers;
			} else if (!modifiers.ctrl) {
				keyTimes.try_emplace(key, scancode, modifiers, getTime());
			}

			if (modifiers.onlyCtrl()) {
				if (key == GLFW_KEY_P) {
					playLocally();
					return;
				}
			}
		}

		if (game) {
			ClientPlayerPtr player = game->getPlayer();

			if (player == nullptr) {
				return;
			}

			if (action == GLFW_PRESS || action == GLFW_REPEAT) {
				if (modifiers.empty() || modifiers.onlyShift()) {
					auto handle = [&](int movement_key, Direction direction) {
						if (key != movement_key)
							return false;

						if (!player->isMoving())
							player->setContinuousInteraction(modifiers.shift, Modifiers(modifiers));

						if (!player->isMoving(direction))
							player->startMoving(direction);

						return true;
					};

					if (handle(GLFW_KEY_W, Direction::Up) || handle(GLFW_KEY_A, Direction::Left) || handle(GLFW_KEY_S, Direction::Down) || handle(GLFW_KEY_D, Direction::Right)) {
						return;
					}
				}

				if (key == GLFW_KEY_SPACE) {
					if (player != nullptr)
						player->jump();
					return;
				}

				if (key == GLFW_KEY_E) {
					if (uiContext.hasDialog<OmniDialog>()) {
						uiContext.removeDialogs<OmniDialog>();
					} else {
						game->interactNextTo(modifiers, Hand::Right);
					}
					return;
				}

				if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER) {
					game->interactNextTo(Modifiers(modifiers), Hand::Right);
					return;
				}

				if (key == GLFW_KEY_Q) {
					game->interactNextTo(modifiers, Hand::Left);
					return;
				}

				if (key == GLFW_KEY_LEFT_BRACKET) {
					player->send(SetHeldItemPacket(true, player->getActiveSlot()));
					return;
				}

				if (key == GLFW_KEY_RIGHT_BRACKET) {
					player->send(SetHeldItemPacket(false, player->getActiveSlot()));
					return;
				}

				if (key == GLFW_KEY_R) {
					game->interactOn(Modifiers(modifiers));
					return;
				}
			}

			if (action == GLFW_PRESS) {
				if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
					if (player->isMoving()) {
						player->send(ContinuousInteractionPacket(player->continuousInteractionModifiers));
					}
					return;
				}

				if (key == GLFW_KEY_F) {
					if (modifiers.onlyCtrl()) {
						autofocus = !autofocus;
					} else if (modifiers.empty()) {
						player->focus(*this, false);
					}
					return;
				}

				if (key == GLFW_KEY_ESCAPE) {
					if (uiContext.removeDialogs<OmniDialog>() == 0) {
						uiContext.addDialog(getOmniDialog());
					}
					return;
				}
			}

			if (action == GLFW_RELEASE) {
				if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
					player->stopContinuousInteraction();
					return;
				}

				auto handle = [&](int released_key, Direction direction) {
					if (key != released_key)
						return false;

					player->stopMoving(direction);
					player->stopContinuousInteraction();

					return true;
				};

				if (handle(GLFW_KEY_W, Direction::Up) || handle(GLFW_KEY_A, Direction::Left) || handle(GLFW_KEY_S, Direction::Down) || handle(GLFW_KEY_D, Direction::Right)) {
					return;
				}
			}
		}
	}

	void Window::mouseButtonCallback(int button, int action, int mods) {
		lastModifiers = Modifiers(mods);

		double x_pos{}, y_pos{};
		glfwGetCursorPos(glfwWindow, &x_pos, &y_pos);
		const auto x = static_cast<int>(std::floor(x_pos));
		const auto y = static_cast<int>(std::floor(y_pos));

		if (action == GLFW_PRESS) {
			heldMouseButton = button;

			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				uiContext.dragStart(x, y);
				clickPosition.emplace(x, y);
			}
		} else if (action == GLFW_RELEASE) {
			heldMouseButton.reset();

			if (button != GLFW_MOUSE_BUTTON_LEFT || clickPosition == std::pair{x, y}) {
				uiContext.click(button, x, y);
			} else {
				uiContext.dragEnd(x, y);
			}
		}
	}

	void Window::mousePositionCallback(int x, int y) {
		if (heldMouseButton == GLFW_MOUSE_BUTTON_LEFT) {
			uiContext.dragUpdate(x, y);
		}
	}

	void Window::scrollCallback(double x_delta, double y_delta) {
		const auto [x, y] = getMouseCoordinates();

		if (uiContext.scroll(x_delta, y_delta, std::floor(x), std::floor(y)))
			return;

		const auto factor = getFactor();
		const auto old_scale = scale;

		if (y_delta < 0)
			scale /= 1.08 * -y_delta;
		else if (y_delta > 0)
			scale *= 1.08 * y_delta;

		const float width = lastWindowSize.first;
		const float height = lastWindowSize.second;

		const auto difference_x = width / old_scale - width / scale;
		const auto side_ratio_x = (x - width / 2.f) / width;
		center.first -= difference_x * side_ratio_x / 8.f * factor;

		const auto difference_y = height / old_scale - height / scale;
		const auto side_ratio_y = (y - height / 2.f) / height;
		center.second -= difference_y * side_ratio_y / 8.f * factor;
	}

	void Window::closeGame() {
		if (game == nullptr)
			return;

		// richPresence.setActivityDetails("Idling");

		// uiContext.removeDialogs();

		// if (dialog)
		// 	dialog->close();

		removeModule();
		game->stopThread();
		game.reset();

		omniDialog.reset();
		uiContext.reset();
	}

	void Window::onGameLoaded() {
		// richPresence.setActivityStartTime(false);
		// richPresence.setActivityDetails("Playing", true);

		uiContext.reset();

		// debugAction->set_state(Glib::Variant<bool>::create(game->debugMode));
		game->initInteractionSets();
		settings.apply(*game);

		game->signalOtherInventoryUpdate().connect([this](const std::shared_ptr<Agent> &owner, InventoryID inventory_id) {
			if (auto has_inventory = std::dynamic_pointer_cast<HasInventory>(owner); has_inventory && has_inventory->getInventory(inventory_id)) {
				auto client_inventory = std::dynamic_pointer_cast<ClientInventory>(has_inventory->getInventory(inventory_id));
				queue([owner, client_inventory](Window &window) {
					if (owner->getGID() == window.getExternalGID()) {
						std::unique_lock<DefaultMutex> lock;
						if (Module *module_ = window.getOmniDialog()->inventoryTab->getModule(lock)) {
							module_->setInventory(client_inventory);
						}
					}
				});
			}
		});

		game->signalPlayerMoneyUpdate().connect([](const PlayerPtr &) {
			// updateMoneyLabel(player->getMoney());
		});

		game->signalFluidUpdate().connect([this](const std::shared_ptr<HasFluids> &has_fluids) {
			queue([has_fluids](Window &window) mutable {
				if (!window.omniDialog)
					return;

				std::unique_lock<DefaultMutex> lock;

				if (Module *module_ = window.omniDialog->inventoryTab->getModule(lock)) {
					std::any data(std::move(has_fluids));
					module_->handleMessage({}, "UpdateFluids", data);
				}
			});
		});

		game->signalEnergyUpdate().connect([this](const std::shared_ptr<HasEnergy> &has_energy) {
			queue([has_energy](Window &window) mutable {
				if (!window.omniDialog)
					return;

				std::unique_lock<DefaultMutex> lock;

				if (Module *module_ = window.omniDialog->inventoryTab->getModule(lock)) {
					std::any data(std::move(has_energy));
					module_->handleMessage({}, "UpdateEnergy", data);
				}
			});
		});

		game->signalVillageUpdate().connect([this](const VillagePtr &village) {
			queue([village](Window &window) mutable {
				if (!window.omniDialog)
					return;

				std::unique_lock<DefaultMutex> lock;

				if (Module *module_ = window.omniDialog->inventoryTab->getModule(lock)) {
					std::any data(std::move(village));
					module_->handleMessage({}, "VillageUpdate", data);
				}
			});
		});

		game->errorCallback = [this] {
			if (game->suppressDisconnectionMessage)
				return;

			queue([](Window &window) {
				// get_display()->beep();
				window.error("Game disconnected.");
				window.closeGame();
			});
		};

		game->startThread();
	}

	bool Window::connect(const std::string &hostname, uint16_t port) {
		closeGame();
		game = std::dynamic_pointer_cast<ClientGame>(Game::create(Side::Client, shared_from_this()));
		auto client = std::make_shared<LocalClient>();
		game->setClient(client);
		try {
			client->connect(hostname, port);
		} catch (const std::exception &err) {
			closeGame();
			error(err.what());
			return false;
		}
		client->weakGame = game;
		game->initEntities();

		// It's assumed that the caller already owns a unique lock on the settings.
		settings.hostname = hostname;
		settings.port = port;

		if (std::filesystem::exists("tokens.json"))
			client->readTokens("tokens.json");
		else
			client->saveTokens("tokens.json");

		activateContext();
		onGameLoaded();

		alert("Connected.");

		if (settings.alertOnConnection) {
			SUCCESS("Connected.");
			// auto success_dialog = std::make_unique<ConnectionSuccessDialog>(*this);
			// success_dialog->signal_submit().connect([this](bool checked) {
			// 	auto lock = settings.uniqueLock();
			// 	settings.alertOnConnection = !checked;
			// });
			// queueDialog(std::move(success_dialog));
		}

		return true;
	}

	void Window::autoConnect() {
		auto lock = settings.uniqueLock();

		if (settings.username.empty()) {
			error("No username set. Try logging in manually first.");
			return;
		}

		if (!connect(settings.hostname, settings.port))
			return;

		LocalClientPtr client = game->getClient();
		const std::string &hostname = client->getHostname();
		if (std::optional<Token> token = client->getToken(hostname, settings.username)) {
			queueBool([client, token, hostname, username = settings.username](Window &window) {
				if (window.game && client && client->isReady()) {
					client->send(LoginPacket(username, *token));
					return true;
				}

				return false;
			});
		} else {
			error("Couldn't find token for user " + settings.username + " on host " + hostname);
		}
	}

	void Window::playLocally() {
		const bool was_running = serverWrapper.isRunning();

		if (game) {
			game->suppressDisconnectionMessage = true;
		}

		size_t seed = 1621;
		if (std::filesystem::exists(".seed")) {
			try {
				seed = parseNumber<size_t>(trim(readFile(".seed")));
				INFO("Using custom seed \e[1m{}\e[22m", seed);
			} catch (const std::exception &err) {
				ERROR("Failed to load seed from .seed: {}", err.what());
			}
		}

		serverWrapper.runInThread(seed);

		if (!serverWrapper.waitUntilRunning(std::chrono::milliseconds(10'000))) {
			error("Server failed to start within 10 seconds.");
			return;
		}

		serverWrapper.save();

		if (was_running) {
			// Ugly hack!
			delay([](Window &window) {
				window.continueLocalConnection();
			}, DEFAULT_CLIENT_TICK_FREQUENCY * 2);
		} else {
			continueLocalConnection();
		}
	}

	void Window::continueLocalConnection() {
		if (!connect("::1", serverWrapper.getPort())) {
			error("Failed to connect to local server.");
			return;
		}

		assert(game != nullptr);
		LocalClientPtr client = game->getClient();

		client->queueForConnect([this, weak = std::weak_ptr(client)] {
			if (LocalClientPtr client = weak.lock()) {
				client->send(LoginPacket(settings.username, serverWrapper.getOmnitoken(), "Heimskr"));
			}
		});
	}

	void Window::handleKeys() {
		std::erase_if(keyTimes, [this](const std::pair<int, KeyInfo> &pair) {
			return glfwGetKey(glfwWindow, pair.first) == GLFW_RELEASE;
		});

		for (auto &[key, info]: keyTimes) {
			auto &[keycode, modifiers, time] = info;
			auto repeat_time = KEY_REPEAT_TIME;
			if (auto iter = CUSTOM_KEY_REPEAT_TIMES.find(key); iter != CUSTOM_KEY_REPEAT_TIMES.end())
				repeat_time = iter->second;
			if (std::chrono::duration_cast<std::chrono::milliseconds>(timeDifference(time)) < repeat_time)
				continue;
			time = getTime();
			keyCallback(key, keycode, GLFW_REPEAT, static_cast<uint8_t>(modifiers));
		}
	}
}
