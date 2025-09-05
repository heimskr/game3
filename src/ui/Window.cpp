#include "Options.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "graphics/RealmRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/Util.h"
#include "lib/PNG.h"
#include "net/DirectLocalClient.h"
#include "net/LocalClient.h"
#include "packet/ContinuousInteractionPacket.h"
#include "packet/LoginPacket.h"
#include "packet/RegisterPlayerPacket.h"
#include "packet/SetHeldItemPacket.h"
#include "threading/ThreadContext.h"
#include "types/Position.h"
#include "ui/dialog/ChatDialog.h"
#include "ui/dialog/ConnectionDialog.h"
#include "ui/dialog/DraggableDialog.h"
#include "ui/dialog/LoginDialog.h"
#include "ui/dialog/MessageDialog.h"
#include "ui/dialog/MinigameDialog.h"
#include "ui/dialog/OmniDialog.h"
#include "ui/dialog/WorldCreatorDialog.h"
#include "ui/dialog/WorldSelectorDialog.h"
#include "ui/module/FluidsModule.h"
#include "ui/module/InventoryModule.h"
#include "ui/module/ModuleFactory.h"
#include "ui/tab/InventoryTab.h"
#include "ui/Constants.h"
#include "ui/Window.h"
#include "ui/Modifiers.h"
#include "ui/GameUI.h"
#include "ui/TitleUI.h"
#include "ui/UI.h"
#include "ui/Window.h"
#include "util/FS.h"
#include "util/Util.h"

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
		scale(8),
		causticsShader(readFile("resources/caustics.frag")),
		waveShader(readFile("resources/wave.frag")),
		colorDodgeShader(readFile("resources/color_dodge.frag")),
		blurShader(readFile("resources/blur.frag")) {
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

			png::image<png::rgba_pixel> icon_png("resources/game3.png");
			std::shared_ptr<uint8_t[]> icon = getRaw(icon_png);
			GLFWimage image{
				.width  = static_cast<int>(icon_png.get_width()),
				.height = static_cast<int>(icon_png.get_height()),
				.pixels = icon.get(),
			};

			glfwSetWindowIcon(glfwWindow, 1, &image);

#ifdef __MINGW32__
			xScale = 1;
			yScale = 1;
#endif

			try {
				settings = boost::json::value_to<ClientSettings>(boost::json::parse(readFile("settings.json")));
			} catch (const std::ios_base::failure &) {}

			settings.apply(uiContext);

			textRenderer.initRenderData();
			uiContext.setUI<TitleUI>();
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

	std::optional<int> Window::getMouseX() const {
		double x{};
		glfwGetCursorPos(glfwWindow, &x, nullptr);
		if (glfwGetError(nullptr) != GLFW_NO_ERROR || !mouseMoved) [[unlikely]] {
			return std::nullopt;
		}
		return static_cast<int>(std::floor(x * xScale));
	}

	std::optional<int> Window::getMouseY() const {
		double y{};
		glfwGetCursorPos(glfwWindow, nullptr, &y);
		if (glfwGetError(nullptr) != GLFW_NO_ERROR || !mouseMoved) [[unlikely]] {
			return std::nullopt;
		}
		return static_cast<int>(std::floor(y * yScale));
	}

	Rectangle Window::inset(int distance) const {
		return uiContext.scissorStack.getTop().rectangle.shrinkAll(distance);
	}

	template <>
	std::optional<std::pair<int, int>> Window::getMouseCoordinates<int>() const {
		double x{}, y{};
		glfwGetCursorPos(glfwWindow, &x, &y);
		if (glfwGetError(nullptr) != GLFW_NO_ERROR || !mouseMoved) [[unlikely]] {
			return std::nullopt;
		}
		return std::optional<std::pair<int, int>>{std::in_place, static_cast<int>(x * xScale), static_cast<int>(y * yScale)};
	}

	template <>
	std::optional<std::pair<float, float>> Window::getMouseCoordinates<float>() const {
		double x{}, y{};
		glfwGetCursorPos(glfwWindow, &x, &y);
		if (glfwGetError(nullptr) != GLFW_NO_ERROR || !mouseMoved) [[unlikely]] {
			return std::nullopt;
		}
		return std::optional<std::pair<float, float>>{std::in_place, static_cast<float>(x * xScale), static_cast<float>(y * yScale)};
	}

	template <>
	std::optional<std::pair<double, double>> Window::getMouseCoordinates<double>() const {
		double x{}, y{};
		glfwGetCursorPos(glfwWindow, &x, &y);
		if (glfwGetError(nullptr) != GLFW_NO_ERROR || !mouseMoved) [[unlikely]] {
			return std::nullopt;
		}
		return std::optional<std::pair<double, double>>{std::in_place, x * xScale, y * yScale};
	}

	void Window::alert(const UString &message, bool do_queue, bool use_markup) {
		(void) use_markup;

		auto action = [message](Window &window) mutable {
			window.activateContext();
			window.uiContext.addDialog(MessageDialog::create(window.uiContext, 1, std::move(message), ButtonsType::None));
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
			auto dialog = MessageDialog::create(window.uiContext, 1, std::move(message), ButtonsType::None);
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

		ERR("Error: {}", message);
	}

	Modifiers Window::getModifiers() const {
		return lastModifiers;
	}

	Position Window::getHoveredPosition() const {
		return {};
	}

	void Window::activateContext() {
		glfwMakeContextCurrent(glfwWindow);
	}

	void Window::saveSettings() {
		std::ofstream ofs("settings.json");
		boost::json::value json;
		{
			auto lock = settings.sharedLock();
			json = boost::json::value_from(settings);
		}

		boost::json::serializer serializer;
		serializer.reset(&json);
		char buffer[512];
		while (!serializer.done()) {
			ofs << serializer.read(buffer);
		}
	}

	bool Window::inBounds(const Position &pos) const {
		Rectangle realmBounds = uiContext.getUI<GameUI>()->realmBounds;
		const auto x = realmBounds.x;
		const auto y = realmBounds.y;
		return x <= pos.column && pos.column < x + realmBounds.width
		    && y <= pos.row    && pos.row    < y + realmBounds.height;
	}

	RendererContext Window::getRendererContext(float delta) {
		return {rectangleRenderer, singleSpriteRenderer, batchSpriteRenderer, textRenderer, circleRenderer, recolor, settings, getXFactor(), getYFactor(), delta};
	}

	void Window::tick(float delta) {
		const auto [width, height] = getDimensions();

		if (!lastWindowSize || width != lastWindowSize->x || height != lastWindowSize->y) {
			uiContext.onResize(width, height);
			lastWindowSize = {width, height};
		}

		handleKeys();

		for (const auto &function: functionQueue.steal()) {
			function(*this);
		}

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

		render(delta);
	}

	void Window::render(float delta) {
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

		if (auto coords = getMouseCoordinates<float>()) {
			auto [x, y] = *coords;
			uiContext.render(x, y, delta);
		} else {
			uiContext.render(-1, -1, delta);
		}

		if (settings.showFPS && runningFPS > 0) {
			renderFPSCounter();
		}
	}

	void Window::keyCallback(int key, int scancode, int action, int raw_modifiers) {
		const Modifiers modifiers(static_cast<uint8_t>(raw_modifiers));

		if (action != CUSTOM_REPEAT) {
			lastModifiers = modifiers;
		}

		if (action == GLFW_PRESS || action == GLFW_RELEASE) {
			const bool held = action == GLFW_PRESS;

			if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
				lastModifiers.shift = held;
			} else if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) {
				lastModifiers.ctrl = held;
			} else if (key == GLFW_KEY_LEFT_ALT || key == GLFW_KEY_RIGHT_ALT) {
				lastModifiers.alt = held;
			} else if (key == GLFW_KEY_LEFT_SUPER || key == GLFW_KEY_RIGHT_SUPER) {
				lastModifiers.super = held;
			}
		}

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
				if (key == GLFW_KEY_O) {
					showWorldSelector();
					return;
				}

				if (key == GLFW_KEY_P) {
					loadLastWorld();
					return;
				}

				if (key == GLFW_KEY_N) {
					if (uiContext.getUI<TitleUI>()) {
						showWorldCreator();
					}
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
						uiContext.getUI<GameUI>()->hideOmniDialog();
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
					if (auto game_ui = uiContext.getUI<GameUI>()) {
						game_ui->toggleOmniDialog();
					}
					return;
				}

				if (GLFW_KEY_0 <= key && key <= GLFW_KEY_9) {
					player->getInventory(0)->setActive(key == GLFW_KEY_0? 9 : key - GLFW_KEY_1);
					return;
				}

				if (key == GLFW_KEY_SLASH) {
					queue([](Window &window) {
						if (auto game_ui = window.uiContext.getUI<GameUI>()) {
							game_ui->getChatDialog()->toggle(false);
						}
					});
					return;
				}

				if (key == GLFW_KEY_BACKSLASH) {
					queue([slash = modifiers.onlyShift()](Window &window) {
						if (auto game_ui = window.uiContext.getUI<GameUI>()) {
							std::shared_ptr<ChatDialog> chat = game_ui->getChatDialog();
							chat->focusInput();
							if (slash) {
								chat->setSlash();
							}
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
		Modifiers modifiers(mods);
		lastModifiers = modifiers;

		const auto coords = getMouseCoordinates<double>();
		if (!coords) {
			return;
		}

		const int x = static_cast<int>(std::floor(coords->first));
		const int y = static_cast<int>(std::floor(coords->second));

		if (action == GLFW_PRESS) {
			heldMouseButton = button;

			bool result = uiContext.mouseDown(button, x, y, modifiers);

			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				if (!result && game) {
					if (ClientPlayerPtr player = game->getPlayer()) {
						player->setFiring(true);
					}
				}

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

			if (game) {
				if (ClientPlayerPtr player = game->getPlayer()) {
					player->setFiring(false);
				}
			}

			bool result = uiContext.mouseUp(button, x, y, modifiers);
			auto displacement = clickPosition? clickPosition->distance({x, y}) : 0;

			if (button != GLFW_MOUSE_BUTTON_LEFT || displacement < uiContext.dragThreshold) {
				result = uiContext.click(button, x, y, modifiers) || result;
			} else {
				result = uiContext.dragEnd(x, y, displacement) || result;
				dragStarted = false;
			}

			if (!result && game) {
				game->click(button, 1, coords->first, coords->second, modifiers);
			}
		}
	}

	void Window::mousePositionCallback(int x, int y) {
		mouseMoved = true;
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
		const auto coords = getMouseCoordinates<double>();
		if (!coords) {
			return;
		}

		const auto [x, y] = *coords;

		if (uiContext.scroll(x_delta, y_delta, std::floor(coords->first), std::floor(coords->second), lastModifiers)) {
			return;
		}

		const auto x_factor = getXFactor();
		const auto y_factor = getYFactor();
		const auto old_scale = scale;

		if (y_delta < 0) {
			scale /= 1 + .08 * -y_delta;
		} else if (y_delta > 0) {
			scale *= 1 + .08 * y_delta;
		}

		if (lastWindowSize) {
			const float width = lastWindowSize->x;
			const float height = lastWindowSize->y;

			const auto difference_x = width / old_scale - width / scale;
			const auto side_ratio_x = (x - width / 2.f) / width;
			center.first -= difference_x * side_ratio_x / 8.f * x_factor;

			const auto difference_y = height / old_scale - height / scale;
			const auto side_ratio_y = (y - height / 2.f) / height;
			center.second -= difference_y * side_ratio_y / 8.f * y_factor;
		}
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

		if (LocalClientPtr client = game->getClient()) {
			client->disconnect();
		}

		connected = false;

		if (auto game_ui = uiContext.getUI<GameUI>()) {
			game_ui->removeModule();
		}

		uiContext.removeDialogs<OmniDialog>();

		game->asyncStopThread(std::chrono::milliseconds(666))->then([self = shared_from_this()] {
			self->queue([](Window &window) {
				window.setGame(nullptr);
				window.serverWrapper.stop();
				window.goToTitle();
			});
		})->oops([self = shared_from_this()](auto) {
			self->queue([](Window &window) {
				window.setGame(nullptr);
				window.serverWrapper.stop();
			});
		});
	}

	void Window::goToTitle() {
		queue([](Window &window) {
			if (auto game_ui = window.uiContext.getUI<GameUI>()) {
				game_ui->omniDialog.reset();
			}
			window.uiContext.reset();
			window.uiContext.setUI<TitleUI>();
		});
	}

	void Window::onGameLoaded() {
		// richPresence.setActivityStartTime(false);
		// richPresence.setActivityDetails("Playing", true);

		connected = true;
		uiContext.reset();

		game->initInteractionSets();
		settings.apply(*game);

		game->signalOtherInventoryUpdate.connect([this](const std::shared_ptr<Agent> &owner, InventoryID inventory_id) {
			if (auto has_inventory = std::dynamic_pointer_cast<HasInventory>(owner); has_inventory && has_inventory->getInventory(inventory_id)) {
				auto client_inventory = std::dynamic_pointer_cast<ClientInventory>(has_inventory->getInventory(inventory_id));
				queue([owner, client_inventory](Window &window) {
					if (auto game_ui = window.uiContext.getUI<GameUI>()) {
						if (owner->getGID() == game_ui->getExternalGID()) {
							std::unique_lock<DefaultMutex> lock;
							if (Module *module_ = game_ui->getOmniDialog()->inventoryTab->getModule(lock)) {
								module_->setInventory(client_inventory);
							}
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
				if (auto game_ui = window.uiContext.getUI<GameUI>()) {
					if (!game_ui->omniDialog) {
						return;
					}

					std::unique_lock<DefaultMutex> lock;

					if (Module *module_ = game_ui->omniDialog->inventoryTab->getModule(lock)) {
						std::any data(std::move(has_fluids));
						module_->handleMessage({}, "UpdateFluids", data);
					}
				}
			});
		});

		game->signalEnergyUpdate.connect([this](const std::shared_ptr<HasEnergy> &has_energy) {
			queue([has_energy](Window &window) mutable {
				if (auto game_ui = window.uiContext.getUI<GameUI>()) {
					if (!game_ui->omniDialog) {
						return;
					}

					std::unique_lock<DefaultMutex> lock;

					if (Module *module_ = game_ui->omniDialog->inventoryTab->getModule(lock)) {
						std::any data(std::move(has_energy));
						module_->handleMessage({}, "UpdateEnergy", data);
					}
				}
			});
		});

		game->signalVillageUpdate.connect([this](const VillagePtr &village) {
			queue([village](Window &window) mutable {
				if (auto game_ui = window.uiContext.getUI<GameUI>()) {
					if (!game_ui->omniDialog) {
						return;
					}

					std::unique_lock<DefaultMutex> lock;

					if (Module *module_ = game_ui->omniDialog->inventoryTab->getModule(lock)) {
						std::any data(std::move(village));
						module_->handleMessage({}, "VillageUpdate", data);
					}
				}
			});
		});

		game->signalChatReceived.connect([this](const PlayerPtr &player, const UString &message) {
			if (auto game_ui = uiContext.getUI<GameUI>()) {
				auto dialog = game_ui->getChatDialog();
				if (player == nullptr) {
					dialog->addMessage(message);
				} else {
					dialog->addMessage(std::format("<{}> {}", player->displayName, message.raw()));
				}
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

	Ref<Promise<void>> Window::connect(const std::string &hostname, uint16_t port, std::shared_ptr<LocalClient> client) {
		closeGame();

		setGame(std::dynamic_pointer_cast<ClientGame>(Game::create(Side::Client, shared_from_this())));

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

		return Promise<void>::now([self = shared_from_this(), hostname, port, client](auto resolve) {
			client->connect(hostname, port);
			client->weakGame = self->game;

			self->game->initEntities();

			self->settings.withUnique([&](auto &) {
				self->settings.hostname = hostname;
				self->settings.port = port;
			});

			if (std::filesystem::exists("tokens.json")) {
				client->readTokens("tokens.json");
			} else {
				client->saveTokens("tokens.json");
			}

			self->activateContext();
			self->onGameLoaded();

			if (self->settings.alertOnConnection) {
				self->alert("Connected.");
			}

			resolve();
		});
	}

	void Window::autoConnect() {
		auto lock = settings.uniqueLock();

		if (settings.username.empty()) {
			error("No username set. Try logging in manually first.");
			return;
		}

		connect(settings.hostname, settings.port)->then([self = shared_from_this()] {
			LocalClientPtr client = self->game->getClient();
			const std::string &hostname = client->getHostname();
			if (std::optional<Token> token = client->getToken(hostname, self->settings.username)) {
				self->queueBool([client, token, hostname, username = self->settings.username](Window &window) {
					if (window.game && client && client->isReady()) {
						client->send(make<LoginPacket>(username, *token));
						return true;
					}

					return false;
				});
			} else {
				self->error(std::format("Couldn't find token for user {} on host {}", self->settings.username, hostname));
			}
		})->oops([self = shared_from_this()](std::exception_ptr exception) {
			try {
				std::rethrow_exception(std::move(exception));
			} catch (const std::exception &err) {
				self->closeGame();
				self->error(err.what(), true, false, [self] { self->goToTitle(); });
			}
		});
	}

	void Window::showWorldSelector() {
		if (!uiContext.hasDialog<WorldSelectorDialog>()) {
			auto dialog = uiContext.emplaceDialog<WorldSelectorDialog>(1);
			uiContext.focusDialog(dialog);
			dialog->signalSubmit.connect([this](const std::filesystem::path &world_path) {
				playLocally(world_path);
			});
		}
	}

	void Window::showWorldCreator() {
		if (!uiContext.hasDialog<WorldCreatorDialog>()) {
			auto dialog = uiContext.emplaceDialog<WorldCreatorDialog>(1);
			uiContext.focusDialog(dialog);
			dialog->signalSubmit.connect([this](const std::filesystem::path &world_path, std::optional<size_t> seed) {
				newWorld(world_path, seed);
			});
		}
	}

	bool Window::loadLastWorld() {
		std::string last_path = settings.withShared([](const ClientSettings &settings) { return settings.lastWorldPath; });
		if (last_path.empty()) {
			return false;
		}

		playLocally(last_path);
		return true;
	}

	void Window::playLocally(std::filesystem::path world_path, std::optional<size_t> seed) {
		if (game) {
			game->suppressDisconnectionMessage = true;
		}

		closeGame();

		if (!seed) {
			if (std::filesystem::exists(".seed")) {
				try {
					seed = parseNumber<size_t>(trim(readFile(".seed")));
					INFO("Using custom seed \e[1m{}\e[22m", *seed);
				} catch (const std::exception &err) {
					ERR("Failed to load seed from .seed: {}", err.what());
				}
			} else {
				seed = 1000;
			}
		}

		if (!world_path.empty()) {
			serverWrapper.worldPath = world_path;
		}

		serverWrapper.onError = [this](const std::exception &exception) {
			error(std::format("ServerWrapper error: {}", exception.what()), true);
			serverWrapper.stop();
			closeGame();
		};

		serverWrapper.runInThread(*seed);

		if (!serverWrapper.waitUntilRunning(std::chrono::milliseconds(10'000))) {
			if (std::exception_ptr caught = serverWrapper.getFailure()) {
				try {
					std::rethrow_exception(caught);
				} catch (const std::exception &err) {
					error(std::format("Server failed to start: {}", err.what()));
				} catch (...) {
					error("Server failed to start due to an error.");
				}
			} else {
				error("Server failed to start within 10 seconds.");
			}
			return;
		}

		settings.setLastWorldPath(world_path);
		serverWrapper.save();
		continueLocalConnection();
	}

	void Window::continueLocalConnection() {
		auto client = std::make_shared<DirectLocalClient>();

		connect("::1", serverWrapper.getPort(), client)->then([this, client] {
			assert(game != nullptr);
			connectedLocally = true;

			// Tie the loop. Or whatever the expression is.
			// What I'm trying to say is that we're doing a funny thing where we make both Direct*Clients point at each other.
			client->setRemote(serverWrapper.getDirectRemoteClient(client));

			client->queueForConnect([this, weak = std::weak_ptr(client)] {
				if (LocalClientPtr client = weak.lock()) {
					queue([this, client](Window &) {
						activateContext();
						auto dialog = make<LoginDialog>(uiContext, 1);

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
		})->oops([self = shared_from_this()](std::exception_ptr exception) {
			try {
				std::rethrow_exception(std::move(exception));
			} catch (const std::exception &err) {
				self->error("Failed to connect to local server.");
				self->closeGame();
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
		auto dialog = make<LoginDialog>(uiContext, 1);

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
		queue([](Window &window) {
			if (window.isConnectedLocally()) {
				window.serverWrapper.stop();
			}

			window.closeGame();
		});
	}

	bool Window::isKeyHeld(int key) const {
		return heldKeys.contains(key);
	}

	void Window::setGame(std::shared_ptr<ClientGame> new_game) {
		game = std::move(new_game);
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

	void Window::renderFPSCounter() {
		auto [width, height] = getDimensions();
		textRenderer.drawOnScreen(std::format("{:.1f} FPS", runningFPS), TextRenderOptions{
			.x = static_cast<double>(width - 10),
			.y = static_cast<double>(height - 10),
			.scaleX = 0.5,
			.scaleY = 0.5,
			.color = "#fff",
			.align = TextAlign::Right,
			.alignTop = false,
			.shadow = "#000",
			.shadowOffset{6, 6},
		});
	}

	void Window::newWorld(const std::filesystem::path &path, std::optional<size_t> seed) {
		auto go = [=, this] {
			playLocally(path, seed);
		};

		if (std::filesystem::exists(path)) {
			if (std::filesystem::is_regular_file(path)) {
				std::string message = std::format("Are you sure you want to overwrite {}?", path.string());
				queue([path, message = std::move(message), go = std::move(go)](Window &window) mutable {
					auto dialog = MessageDialog::create(window.uiContext, 1, std::move(message), ButtonsType::NoYes);
					dialog->setTitle("Overwrite?");
					dialog->signalSubmit.connect([path = std::move(path), go = std::move(go)](bool response) {
						if (response) {
							std::filesystem::remove(path);
							go();
						}
					});
					window.uiContext.addDialog(std::move(dialog));
				});
			} else {
				error(std::format("Can't overwrite {}.", path.string()));
			}
		} else {
			go();
		}
	}
}
