#include "Options.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "graphics/RendererContext.h"
#include "graphics/Tileset.h"
#include "net/DirectLocalClient.h"
#include "net/LocalClient.h"
#include "packet/ContinuousInteractionPacket.h"
#include "packet/LoginPacket.h"
#include "packet/RegisterPlayerPacket.h"
#include "packet/SetHeldItemPacket.h"
#include "types/Position.h"
#include "ui/gl/dialog/ChatDialog.h"
#include "ui/gl/dialog/ConnectionDialog.h"
#include "ui/gl/dialog/DraggableDialog.h"
#include "ui/gl/dialog/LoginDialog.h"
#include "ui/gl/dialog/MessageDialog.h"
#include "ui/gl/dialog/MinigameDialog.h"
#include "ui/gl/dialog/OmniDialog.h"
#include "ui/gl/dialog/TopDialog.h"
#include "ui/gl/module/FluidsModule.h"
#include "ui/gl/module/InventoryModule.h"
#include "ui/gl/module/ModuleFactory.h"
#include "ui/gl/tab/InventoryTab.h"
#include "ui/gl/Constants.h"
#include "ui/Window.h"
#include "ui/Modifiers.h"
#include "ui/Window.h"
#include "util/FS.h"
#include "util/Util.h"

#include "minigame/Breakout.h"

#include <fstream>

namespace Game3 {
	namespace {
		constexpr std::chrono::milliseconds KEY_REPEAT_INTERVAL{100};
		constexpr std::chrono::milliseconds ARROW_TIME{100};
		constexpr std::chrono::milliseconds INTERACT_TIME{250};
		constexpr std::chrono::milliseconds JUMP_TIME{50};
		constexpr std::chrono::milliseconds SLOW_TIME{1'000};
		constexpr std::chrono::milliseconds FOREVER{1'000'000'000};

		std::map<guint, std::chrono::milliseconds> CUSTOM_KEY_REPEAT_INTERVALS{
			{GLFW_KEY_UP,            ARROW_TIME},
			{GLFW_KEY_DOWN,          ARROW_TIME},
			{GLFW_KEY_LEFT,          ARROW_TIME},
			{GLFW_KEY_RIGHT,         ARROW_TIME},
			{GLFW_KEY_Q,             INTERACT_TIME},
			{GLFW_KEY_E,             INTERACT_TIME},
			{GLFW_KEY_LEFT_BRACKET,  INTERACT_TIME},
			{GLFW_KEY_RIGHT_BRACKET, INTERACT_TIME},
			{GLFW_KEY_R,             INTERACT_TIME},
			{GLFW_KEY_O,             INTERACT_TIME},
			{GLFW_KEY_ENTER,         INTERACT_TIME},
			{GLFW_KEY_SPACE,         JUMP_TIME},
			{GLFW_KEY_G,             SLOW_TIME},
			{GLFW_KEY_0,             SLOW_TIME},
			{GLFW_KEY_1,             SLOW_TIME},
			{GLFW_KEY_2,             SLOW_TIME},
			{GLFW_KEY_3,             SLOW_TIME},
			{GLFW_KEY_4,             SLOW_TIME},
			{GLFW_KEY_5,             SLOW_TIME},
			{GLFW_KEY_6,             SLOW_TIME},
			{GLFW_KEY_7,             SLOW_TIME},
			{GLFW_KEY_8,             SLOW_TIME},
			{GLFW_KEY_9,             SLOW_TIME},
			{GLFW_KEY_ESCAPE,        FOREVER},
		};

		constexpr int CUSTOM_REPEAT = 666;

		constexpr bool IS_REPEAT(int action) {
			return action == GLFW_REPEAT || action == CUSTOM_REPEAT;
		}
	}

	Window::Window(GLFWwindow &glfw_window):
		glfwWindow(&glfw_window),
		scale(8) {
			glfwSetWindowUserPointer(glfwWindow, this);

			glfwSetKeyCallback(glfwWindow, +[](GLFWwindow *glfw_window, int key, int scancode, int action, int mods) {
				reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw_window))->keyCallback(key, scancode, action, mods);
			});

			glfwSetCharModsCallback(glfwWindow, +[](GLFWwindow *glfw_window, uint32_t codepoint, int mods) {
				reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw_window))->charCallback(codepoint, mods);
			});

			glfwSetMouseButtonCallback(glfwWindow, +[](GLFWwindow *glfw_window, int button, int action, int mods) {
				reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw_window))->mouseButtonCallback(button, action, mods);
			});

			glfwSetCursorPosCallback(glfwWindow, +[](GLFWwindow *glfw_window, double x_pos, double y_pos) {
				Window &window = *reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw_window));
				const auto x = static_cast<int>(std::floor(x_pos * window.xScale));
				const auto y = static_cast<int>(std::floor(y_pos * window.yScale));
				window.mousePositionCallback(x, y);
			});

			glfwSetScrollCallback(glfwWindow, +[](GLFWwindow *glfw_window, double x, double y) {
				reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw_window))->scrollCallback(x, y);
			});

			glfwSetWindowContentScaleCallback(glfwWindow, +[](GLFWwindow *glfw_window, float x_scale, float y_scale) {
				reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw_window))->contentScaleCallback(x_scale, y_scale);
			});

			glfwGetWindowContentScale(glfwWindow, &xScale, &yScale);

			try {
				settings = nlohmann::json::parse(readFile("settings.json"));
			} catch (const std::ios_base::failure &) {}

			settings.apply();

			fbo.init();
			textRenderer.initRenderData();
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
		int width{};
		glfwGetWindowSize(glfwWindow, &width, nullptr);
		return width * xScale;
	}

	int Window::getHeight() const {
		int height{};
		glfwGetWindowSize(glfwWindow, nullptr, &height);
		return height * yScale;
	}

	std::pair<int, int> Window::getDimensions() const {
		int width{}, height{};
		glfwGetWindowSize(glfwWindow, &width, &height);
		return {static_cast<int>(width * xScale), static_cast<int>(height * yScale)};
	}

	float Window::getXFactor() const {
		return xScale;
	}

	float Window::getYFactor() const {
		return yScale;
	}

	int Window::getMouseX() const {
		double x{};
		glfwGetCursorPos(glfwWindow, &x, nullptr);
		return static_cast<int>(std::floor(x * xScale));
	}

	int Window::getMouseY() const {
		double y{};
		glfwGetCursorPos(glfwWindow, nullptr, &y);
		return static_cast<int>(std::floor(y * yScale));
	}

	template <>
	std::pair<int, int> Window::getMouseCoordinates() const {
		double x{}, y{};
		glfwGetCursorPos(glfwWindow, &x, &y);
		return {static_cast<int>(std::floor(x * xScale)), static_cast<int>(std::floor(y * yScale))};
	}

	template <>
	std::pair<float, float> Window::getMouseCoordinates() const {
		double x{}, y{};
		glfwGetCursorPos(glfwWindow, &x, &y);
		return {static_cast<float>(x * xScale), static_cast<float>(y * yScale)};
	}

	template <>
	std::pair<double, double> Window::getMouseCoordinates() const {
		double x{}, y{};
		glfwGetCursorPos(glfwWindow, &x, &y);
		return {x * xScale, y * yScale};
	}

	const std::shared_ptr<OmniDialog> & Window::getOmniDialog() {
		if (!omniDialog) {
			omniDialog = make<OmniDialog>(uiContext, UI_SCALE);
		}
		return omniDialog;
	}

	const std::shared_ptr<ChatDialog> & Window::getChatDialog() {
		if (!chatDialog) {
			chatDialog = make<ChatDialog>(uiContext);
		}
		return chatDialog;
	}

	const std::shared_ptr<TopDialog> & Window::getTopDialog() {
		if (!topDialog) {
			topDialog = make<TopDialog>(uiContext);
		}
		return topDialog;
	}

	void Window::showOmniDialog() {
		if (!uiContext.hasDialog<OmniDialog>()) {
			uiContext.addDialog(getOmniDialog());
			uiContext.addDialog(getTopDialog());
		}
	}

	void Window::hideOmniDialog() {
		uiContext.removeDialogs<OmniDialog, TopDialog>();
	}

	void Window::toggleOmniDialog() {
		if (uiContext.hasDialog<OmniDialog>()) {
			hideOmniDialog();
		} else {
			showOmniDialog();
		}
	}

	void Window::openModule(const Identifier &module_id, const std::any &argument) {
		assert(game != nullptr);

		auto &registry = game->registry<ModuleFactoryRegistry>();

		if (auto factory = registry[module_id]) {
			getOmniDialog();
			omniDialog->inventoryTab->setModule((*factory)(game, argument));
			omniDialog->activeTab = omniDialog->inventoryTab;
			showOmniDialog();
			return;
		}

		WARN("Couldn't find module {}", module_id);
	}

	void Window::removeModule() {
		if (omniDialog) {
			omniDialog->inventoryTab->removeModule();
		}
	}

	void Window::alert(const UString &message, bool do_queue, bool use_markup) {
		(void) use_markup;

		auto action = [message](Window &window) mutable {
			window.activateContext();
			window.uiContext.addDialog(MessageDialog::create(window.uiContext, std::move(message), ButtonsType::None));
		};

		if (do_queue) {
			queue(std::move(action));
		} else {
			action(*this);
		}

		INFO("{}", message);
	}

	void Window::error(const UString &message, bool do_queue, bool use_markup, std::function<void()> on_close) {
		(void) use_markup;

		auto action = [message, on_close = std::move(on_close)](Window &window) mutable {
			window.activateContext();
			auto dialog = MessageDialog::create(window.uiContext, std::move(message), ButtonsType::None);
			dialog->setTitle("Error");
			if (on_close) {
				dialog->signalClose.connect(std::move(on_close));
			}
			window.uiContext.addDialog(std::move(dialog));
		};

		if (do_queue) {
			queue(std::move(action));
		} else {
			action(*this);
		}

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
				if (std::optional<Buffer> response = module_->handleMessage({}, "GetAgentGID", empty)) {
					return response->take<GlobalID>();
				}
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
		return {rectangleRenderer, singleSpriteRenderer, batchSpriteRenderer, textRenderer, circleRenderer, recolor, settings, getXFactor(), getYFactor()};
	}

	void Window::tick() {
		const auto [width, height] = getDimensions();

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

		boolFunctions.withUnique([this](std::list<std::function<bool (Game3::Window &)>> &bool_functions) {
			std::erase_if(bool_functions, [this](const auto &function) {
				return function(*this);
			});
		});

		drawGL();
	}

	Breakout breakout = [] {
		Breakout out;
		out.setSize(600, 600);
		out.reset();
		return out;
	}();

	void Window::drawGL() {
		const float x_factor = getXFactor();
		const float y_factor = getYFactor();
		auto [width, height] = getDimensions();

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

		if (game != nullptr && game->getPlayer() != nullptr) {
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

			if (mainTexture.getWidth() != width || mainTexture.getHeight() != height) {
				mainTexture.initRGBA(width, height, GL_NEAREST);
				staticLightingTexture.initRGBA(x_static_size, y_static_size, GL_NEAREST);
				dynamicLightingTexture.initRGBA(width, height, GL_NEAREST);
				scratchTexture.initRGBA(width, height, GL_NEAREST);

				GL::FBOBinder binder = fbo.getBinder();
				dynamicLightingTexture.useInFB();
				GL::clear(1, 1, 1);

				if (realm) {
					realm->queueStaticLightingTexture();
				}
			}

			bool do_lighting = settings.withShared([&](auto &settings) {
				return settings.renderLighting;
			});

			if (realm) {
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
						.scaleX = 1. / x_factor,
						.scaleY = 1. / y_factor,
						.viewportX = -static_cast<double>(x_factor),
						.viewportY = -static_cast<double>(y_factor),
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
		} else {
			static float hue = 0;
			static TexturePtr gangblanc = cacheTexture("resources/gangblanc.png");
			static bool has_been_nonzero = false;

			const auto [mouse_x, mouse_y] = getMouseCoordinates<double>();

			if (has_been_nonzero || mouse_x != 0 || mouse_y != 0) {
				singleSpriteRenderer.drawOnScreen(gangblanc, RenderOptions{
					.x = mouse_x,
					.y = mouse_y,
					.scaleX = 16,
					.scaleY = 16,
					.invertY = false,
				});
			}

			if (!has_been_nonzero && (mouse_x != 0 || mouse_y != 0)) {
				has_been_nonzero = true;
			}

			OKHsv hsv{hue += 0.001, 1, 1, 1};
			constexpr int i_max = 32;
			constexpr double offset_factor = 2;
			constexpr double x_offset = offset_factor * i_max;

			for (int i = 0; i < i_max; ++i) {
				const double offset = offset_factor * (i_max - i);
				textRenderer.drawOnScreen("game3", TextRenderOptions{
					.x = getWidth() / 2.0 + (offset - x_offset) / 2,
					.y = 64.0 + offset / 2,
					.scaleX = 2.5,
					.scaleY = 2.5,
					.color = hsv.convert<Color>(),
					.align = TextAlign::Center,
					.alignTop = true,
				});

				hsv.hue += 0.5 / i_max;
			}
		}

		uiContext.render(getMouseX(), getMouseY());

		if (settings.showFPS && runningFPS > 0) {
			textRenderer.drawOnScreen(std::format("{:.1f} FPS", runningFPS), TextRenderOptions{
				.x = static_cast<double>(width - 10),
				.y = static_cast<double>(height - 10),
				.scaleX = 0.5,
				.scaleY = 0.5,
				.color = Color{"#ffffff"},
				.align = TextAlign::Right,
				.alignTop = false,
				.shadow = Color{"#000000"},
				.shadowOffset{6, 6},
			});
		}
	}

	void Window::keyCallback(int key, int scancode, int action, int raw_modifiers) {
		const Modifiers modifiers(static_cast<uint8_t>(raw_modifiers));
		lastModifiers = modifiers;

		if (action == GLFW_PRESS) {
			heldKeys.insert(key);
		} else if (action == GLFW_RELEASE) {
			heldKeys.erase(key);
		}

		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			if (auto iter = keyTimes.find(key); iter != keyTimes.end()) {
				iter->second.modifiers = modifiers;
			} else if (!modifiers.ctrl && action == GLFW_PRESS) {
				keyTimes.try_emplace(key, scancode, modifiers, getTime());
			}

			if (modifiers.onlyCtrl() && action == GLFW_PRESS) {
				if (key == GLFW_KEY_P) {
					playLocally();
					return;
				}
			}
		}

		if (action == GLFW_RELEASE) {
			keyTimes.erase(key);
		} else if (WidgetPtr focused = uiContext.getFocusedWidget()) {
			if (focused->keyPressed(key, modifiers, IS_REPEAT(action))) {
				return;
			}
		} else if (DialogPtr focused = uiContext.getFocusedDialog()) {
			if (focused->keyPressed(key, modifiers, IS_REPEAT(action))) {
				return;
			}
		}

		if (game) {
			ClientPlayerPtr player = game->getPlayer();

			if (player == nullptr) {
				return;
			}

			if (action == GLFW_PRESS || action == CUSTOM_REPEAT) {
				if (modifiers.empty() || modifiers.onlyShift()) {
					auto handle = [&](int movement_key, Direction direction) {
						if (key != movement_key) {
							return false;
						}

						if (!player->isMoving()) {
							player->setContinuousInteraction(modifiers.shift, Modifiers(modifiers));
						}

						if (!player->isMoving(direction)) {
							player->startMoving(direction);
						}

						return true;
					};

					if (handle(GLFW_KEY_W, Direction::Up) || handle(GLFW_KEY_A, Direction::Left) || handle(GLFW_KEY_S, Direction::Down) || handle(GLFW_KEY_D, Direction::Right)) {
						return;
					}
				}

				if (key == GLFW_KEY_SPACE) {
					if (player != nullptr) {
						player->jump();
					}
					return;
				}

				if (key == GLFW_KEY_E) {
					if (uiContext.hasDialog<OmniDialog>()) {
						hideOmniDialog();
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
					player->send(make<SetHeldItemPacket>(true, player->getActiveSlot()));
					return;
				}

				if (key == GLFW_KEY_RIGHT_BRACKET) {
					player->send(make<SetHeldItemPacket>(false, player->getActiveSlot()));
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
						player->send(make<ContinuousInteractionPacket>(player->continuousInteractionModifiers));
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
					toggleOmniDialog();
					return;
				}

				if (GLFW_KEY_0 <= key && key <= GLFW_KEY_9) {
					player->getInventory(0)->setActive(key == GLFW_KEY_0? 9 : key - GLFW_KEY_1);
					return;
				}

				if (key == GLFW_KEY_UP) {
					player->face(Direction::Up);
					return;
				}

				if (key == GLFW_KEY_RIGHT) {
					player->face(Direction::Right);
					return;
				}

				if (key == GLFW_KEY_DOWN) {
					player->face(Direction::Down);
					return;
				}

				if (key == GLFW_KEY_LEFT) {
					player->face(Direction::Left);
					return;
				}

				if (key == GLFW_KEY_SLASH) {
					queue([](Window &window) {
						window.getChatDialog()->toggle(false);
					});
					return;
				}

				if (key == GLFW_KEY_BACKSLASH) {
					queue([slash = modifiers.onlyShift()](Window &window) {
						std::shared_ptr<ChatDialog> chat = window.getChatDialog();
						chat->focusInput();
						if (slash) {
							chat->setSlash();
						}
					});
					return;
				}
			}

			if (action == GLFW_RELEASE) {
				if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
					player->stopContinuousInteraction();
					return;
				}

				auto handle = [&](int released_key, Direction direction) {
					if (key != released_key) {
						return false;
					}

					player->stopMoving(direction);
					return true;
				};

				if (handle(GLFW_KEY_W, Direction::Up) || handle(GLFW_KEY_A, Direction::Left) || handle(GLFW_KEY_S, Direction::Down) || handle(GLFW_KEY_D, Direction::Right)) {
					return;
				}
			}
		}

		if (action == GLFW_PRESS) {
			uiContext.keyPressed(key, modifiers, false);
		}
	}

	void Window::charCallback(uint32_t codepoint, int raw_modifiers) {
		if (WidgetPtr focused = uiContext.getFocusedWidget()) {
			focused->charPressed(codepoint, Modifiers(static_cast<uint8_t>(raw_modifiers)));
		}
	}

	void Window::mouseButtonCallback(int button, int action, int mods) {
		lastModifiers = Modifiers(mods);

		const auto [x_pos, y_pos] = getMouseCoordinates<double>();
		const auto x = static_cast<int>(std::floor(x_pos));
		const auto y = static_cast<int>(std::floor(y_pos));

		if (action == GLFW_PRESS) {
			heldMouseButton = button;

			uiContext.mouseDown(button, x, y);

			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				dragStarted = false;
				clickPosition.emplace(x, y);
			} else if (button == GLFW_MOUSE_BUTTON_4 || button == GLFW_MOUSE_BUTTON_5) {
				if (game) {
					if (ClientPlayerPtr player = game->getPlayer()) {
						InventoryPtr inventory = player->getInventory(0);
						if (button == GLFW_MOUSE_BUTTON_4) {
							inventory->prevSlot();
						} else {
							inventory->nextSlot();
						}
					}
				}
			}
		} else if (action == GLFW_RELEASE) {
			heldMouseButton.reset();

			bool result = uiContext.mouseUp(button, x, y);

			if (button != GLFW_MOUSE_BUTTON_LEFT || clickPosition == std::pair{x, y}) {
				result = uiContext.click(button, x, y) || result;
			} else {
				result = uiContext.dragEnd(x, y) || result;
				dragStarted = false;
			}

			if (!result && game) {
				game->click(button, 1, x_pos, y_pos, lastModifiers);
			}
		}
	}

	void Window::mousePositionCallback(int x, int y) {
		if (heldMouseButton == GLFW_MOUSE_BUTTON_LEFT) {
			if (dragStarted) {
				uiContext.dragUpdate(x, y);
			} else {
				uiContext.dragStart(x, y);
				dragStarted = true;
			}
		}
	}

	void Window::scrollCallback(double x_delta, double y_delta) {
		const auto [x, y] = getMouseCoordinates<double>();

		if (uiContext.scroll(x_delta, y_delta, std::floor(x), std::floor(y))) {
			return;
		}

		const auto x_factor = getXFactor();
		const auto y_factor = getYFactor();
		const auto old_scale = scale;

		if (y_delta < 0) {
			scale /= 1.08 * -y_delta;
		} else if (y_delta > 0) {
			scale *= 1.08 * y_delta;
		}

		const float width = lastWindowSize.first;
		const float height = lastWindowSize.second;

		const auto difference_x = width / old_scale - width / scale;
		const auto side_ratio_x = (x - width / 2.f) / width;
		center.first -= difference_x * side_ratio_x / 8.f * x_factor;

		const auto difference_y = height / old_scale - height / scale;
		const auto side_ratio_y = (y - height / 2.f) / height;
		center.second -= difference_y * side_ratio_y / 8.f * y_factor;
	}

	void Window::contentScaleCallback(float x_scale, float y_scale) {
		xScale = x_scale;
		yScale = y_scale;
	}

	void Window::closeGame() {
		if (game == nullptr) {
			return;
		}

		// richPresence.setActivityDetails("Idling");

		connected = false;
		removeModule();
		game->stopThread();
		game.reset();
		serverWrapper.stop();
		goToTitle();
	}

	void Window::goToTitle() {
		omniDialog.reset();
		uiContext.reset();
		uiContext.emplaceDialog<ConnectionDialog>();
	}

	void Window::onGameLoaded() {
		// richPresence.setActivityStartTime(false);
		// richPresence.setActivityDetails("Playing", true);

		connected = true;
		uiContext.reset();
		uiContext.addDialog(getChatDialog());

		game->initInteractionSets();
		settings.apply(*game);

		game->signalOtherInventoryUpdate.connect([this](const std::shared_ptr<Agent> &owner, InventoryID inventory_id) {
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

		game->signalPlayerMoneyUpdate.connect([](const PlayerPtr &) {
			// updateMoneyLabel(player->getMoney());
		});

		game->signalFluidUpdate.connect([this](const std::shared_ptr<HasFluids> &has_fluids) {
			queue([has_fluids](Window &window) mutable {
				if (!window.omniDialog) {
					return;
				}

				std::unique_lock<DefaultMutex> lock;

				if (Module *module_ = window.omniDialog->inventoryTab->getModule(lock)) {
					std::any data(std::move(has_fluids));
					module_->handleMessage({}, "UpdateFluids", data);
				}
			});
		});

		game->signalEnergyUpdate.connect([this](const std::shared_ptr<HasEnergy> &has_energy) {
			queue([has_energy](Window &window) mutable {
				if (!window.omniDialog) {
					return;
				}

				std::unique_lock<DefaultMutex> lock;

				if (Module *module_ = window.omniDialog->inventoryTab->getModule(lock)) {
					std::any data(std::move(has_energy));
					module_->handleMessage({}, "UpdateEnergy", data);
				}
			});
		});

		game->signalVillageUpdate.connect([this](const VillagePtr &village) {
			queue([village](Window &window) mutable {
				if (!window.omniDialog) {
					return;
				}

				std::unique_lock<DefaultMutex> lock;

				if (Module *module_ = window.omniDialog->inventoryTab->getModule(lock)) {
					std::any data(std::move(village));
					module_->handleMessage({}, "VillageUpdate", data);
				}
			});
		});

		game->signalChatReceived.connect([this](const PlayerPtr &player, const UString &message) {
			auto dialog = getChatDialog();
			if (player == nullptr) {
				dialog->addMessage(message);
			} else {
				dialog->addMessage(std::format("<{}> {}", player->displayName, message.raw()));
			}
		});

		game->errorCallback = [this] {
			if (game->suppressDisconnectionMessage) {
				return;
			}

			queue([](Window &window) {
				window.closeGame();
				window.error("Game disconnected.");
			});
		};

		game->startThread();
	}

	bool Window::connect(const std::string &hostname, uint16_t port, std::shared_ptr<LocalClient> client) {
		closeGame();

		game = std::dynamic_pointer_cast<ClientGame>(Game::create(Side::Client, shared_from_this()));

		if (client == nullptr) {
			client = std::make_shared<LocalClient>();
		}

		client->onError = [this](const asio::error_code &errc) {
			queue([errc](Window &window) {
				window.closeGame();
				window.error(std::format("{} ({})", errc.message(), errc.value()));
			});
		};

		game->setClient(client);

		try {
			client->connect(hostname, port);
		} catch (const std::exception &err) {
			closeGame();
			error(err.what(), true, false, [this] { goToTitle(); });
			return false;
		}

		client->weakGame = game;

		game->initEntities();

		settings.withUnique([&](auto &) {
			settings.hostname = hostname;
			settings.port = port;
		});

		if (std::filesystem::exists("tokens.json")) {
			client->readTokens("tokens.json");
		} else {
			client->saveTokens("tokens.json");
		}

		activateContext();
		onGameLoaded();

		if (settings.alertOnConnection) {
			alert("Connected.");
		}

		return true;
	}

	void Window::autoConnect() {
		auto lock = settings.uniqueLock();

		if (settings.username.empty()) {
			error("No username set. Try logging in manually first.");
			return;
		}

		if (!connect(settings.hostname, settings.port)) {
			return;
		}

		LocalClientPtr client = game->getClient();
		const std::string &hostname = client->getHostname();
		if (std::optional<Token> token = client->getToken(hostname, settings.username)) {
			queueBool([client, token, hostname, username = settings.username](Window &window) {
				if (window.game && client && client->isReady()) {
					client->send(make<LoginPacket>(username, *token));
					return true;
				}

				return false;
			});
		} else {
			error("Couldn't find token for user " + settings.username + " on host " + hostname);
		}
	}

	void Window::playLocally() {
		if (game) {
			game->suppressDisconnectionMessage = true;
		}

		closeGame();

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
		continueLocalConnection();
	}

	void Window::continueLocalConnection() {
		auto client = std::make_shared<DirectLocalClient>();

		if (!connect("::1", serverWrapper.getPort(), client)) {
			error("Failed to connect to local server.");
			closeGame();
			return;
		}

		assert(game != nullptr);
		connectedLocally = true;

		// Tie the loop. Or whatever the expression is.
		// What I'm trying to say is that we're doing a funny thing where we make both Direct*Clients point at each other.
		client->setRemote(serverWrapper.getDirectRemoteClient(client));

		client->queueForConnect([this, weak = std::weak_ptr(client)] {
			if (LocalClientPtr client = weak.lock()) {
				queue([this, client](Window &) {
					activateContext();
					auto dialog = make<LoginDialog>(uiContext);

					dialog->signalSubmit.connect([this, client](const UString &username, const UString &display_name) {
						client->send(make<LoginPacket>(username.raw(), serverWrapper.getOmnitoken(), display_name.raw()));
					});

					dialog->signalDismiss.connect([this] {
						queue([this](Window &) {
							closeGame();
						});
					});

					uiContext.addDialog(std::move(dialog));
				});
			}
		});
	}

	void Window::feedFPS(double fps) {
		if (!settings.showFPS) {
			return;
		}

		fpses.push_back(fps);
		runningSum += fps;

		if (const std::size_t smoothing = settings.fpsSmoothing; smoothing < fpses.size()) {
			if (smoothing + 1 == fpses.size()) {
				runningSum -= fpses.front();
				fpses.pop_front();
			} else {
				auto end = fpses.begin() + fpses.size() - smoothing;
				runningSum -= std::reduce(fpses.begin(), end);
				fpses.erase(fpses.begin(), end);
			}
		}

		// To prevent floating point inaccuracy from accumulating too much.
		if (++fpsCountup >= 1'000) {
			runningSum = std::reduce(fpses.begin(), fpses.end());
		}

		runningFPS = runningSum / fpses.size();
	}

	void Window::showLoginAndRegisterDialogs(const std::string &hostname) {
		if (!game) {
			return;
		}

		LocalClientPtr client = game->getClient();

		if (!client) {
			return;
		}

		activateContext();
		auto dialog = make<LoginDialog>(uiContext);

		dialog->signalSubmit.connect([this, client, hostname](const UString &username, const UString &display_name) {
			if (std::optional<Token> token = client->getToken(hostname, username.raw())) {
				client->send(make<LoginPacket>(username.raw(), *token));
			} else if (display_name.empty()) {
				queue([this, username](Window &) {
					closeGame();
					error(std::format("Token not found for user {} and no display name given.", username), true, false, [this] { goToTitle(); });
				});
			} else {
				client->send(make<RegisterPlayerPacket>(username.raw(), display_name.raw()));
			}
		});

		dialog->signalDismiss.connect([this] {
			queue([this](Window &) {
				closeGame();
			});
		});

		uiContext.addDialog(std::move(dialog));
	}

	bool Window::isConnectedLocally() const {
		return connectedLocally;
	}

	bool Window::isConnected() const {
		return connected;
	}

	void Window::disconnect() {
		if (isConnectedLocally()) {
			serverWrapper.stop();
		}

		closeGame();
	}

	bool Window::isKeyHeld(int key) const {
		return heldKeys.contains(key);
	}

	void Window::handleKeys() {
		std::erase_if(keyTimes, [this](const std::pair<int, KeyInfo> &pair) {
			return glfwGetKey(glfwWindow, pair.first) == GLFW_RELEASE;
		});

		for (auto &[key, info]: keyTimes) {
			auto &[keycode, modifiers, time] = info;

			auto repeat_interval = KEY_REPEAT_INTERVAL;
			if (auto iter = CUSTOM_KEY_REPEAT_INTERVALS.find(key); iter != CUSTOM_KEY_REPEAT_INTERVALS.end()) {
				repeat_interval = iter->second;
			}

			if (std::chrono::duration_cast<std::chrono::milliseconds>(timeDifference(time)) < repeat_interval) {
				continue;
			}

			time = getTime();
			keyCallback(key, keycode, CUSTOM_REPEAT, static_cast<uint8_t>(modifiers));
		}
	}
}
