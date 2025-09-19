#pragma once

#include "client/ClientSettings.h"
#include "client/ServerWrapper.h"
#include "data/Identifier.h"
#include "game/Forward.h"
#include "graphics/BatchSpriteRenderer.h"
#include "graphics/CircleRenderer.h"
#include "graphics/GL.h"
#include "graphics/Multiplier.h"
#include "graphics/Overlayer.h"
#include "graphics/PathmapTextureCache.h"
#include "graphics/Recolor.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/Reshader.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/TextRenderer.h"
#include "math/Rectangle.h"
#include "threading/Lockable.h"
#include "threading/MTQueue.h"
#include "threading/Promise.h"
#include "types/UString.h"
#include "ui/UIContext.h"
#include "util/Concepts.h"

#include <any>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <set>
#include <string>

struct GLFWwindow;

namespace Game3 {
	class Agent;
	class ChatDialog;
	class ClientInventory;
	class HasFluids;
	class LocalClient;
	class TopDialog;
	class Window;
	struct Modifiers;
	struct Position;
	struct RendererContext;

	class Window: public std::enable_shared_from_this<Window> {
		public:
			GLFWwindow *glfwWindow = nullptr;
			Lockable<ClientSettings> settings;
			std::pair<double, double> center{0, 0};
			double scale{};
			float magic = 8;
			float xScale = 1.0;
			float yScale = 1.0;
			bool autofocus = true;
			UIContext uiContext{*this};
			BatchSpriteRenderer  batchSpriteRenderer{*this};
			SingleSpriteRenderer singleSpriteRenderer{*this};
			TextRenderer textRenderer{*this};
			RectangleRenderer rectangleRenderer{*this};
			CircleRenderer circleRenderer{*this};
			Recolor recolor{*this};
			Reshader causticsShader;
			Reshader waveShader;
			Reshader colorDodgeShader;
			Reshader blurShader;
			Multiplier multiplier;
			Overlayer overlayer;
			std::set<int> heldKeys;
			std::optional<std::chrono::system_clock::time_point> lastRenderTime;

			Window(GLFWwindow &);

			void queue(std::move_only_function<void(Window &)>);
			/* If the given function returns false, it won't be removed from the queue. */
			void queueBool(std::move_only_function<bool(Window &)>);
			void delay(std::move_only_function<void(Window &)>, uint32_t count = 1);

			ClientGamePtr getGame() const;
			int getWidth() const;
			int getHeight() const;
			std::pair<int, int> getDimensions() const;
			float getXFactor() const;
			float getYFactor() const;
			std::optional<int> getMouseX() const;
			std::optional<int> getMouseY() const;
			Rectangle inset(int distance = 0) const;

			template <Numeric T>
			std::optional<std::pair<T, T>> getMouseCoordinates() const;

			/** Displays an alert. This will reset the dialog pointer. If you need to use this inside a dialog's code, use delay or queue. */
			void alert(const UString &message, bool do_queue = true, bool use_markup = false);

			/** Displays an error message. (See alert.) */
			void error(const UString &message, bool do_queue = true, bool use_markup = false, std::function<void()> on_close = {});

			Modifiers getModifiers() const;
			Position getHoveredPosition() const;

			void activateContext();

			/** Acquires a shared lock on the settings. */
			void saveSettings();

			bool inBounds(const Position &) const;
			RendererContext getRendererContext(float delta);

			void tick(float delta);
			void render(float delta);
			[[nodiscard]] Ref<Promise<void>> closeGame();
			void goToTitle();
			[[nodiscard]] Ref<Promise<void>> connect(const std::string &hostname, uint16_t port, std::shared_ptr<LocalClient> = nullptr);
			void showWorldSelector();
			void showWorldCreator();
			/** Resolves to whether there was a last loaded world to load. */
			Ref<Promise<bool>> loadLastWorld();
			[[nodiscard]] Ref<Promise<void>> playLocally(std::filesystem::path world_path, std::optional<size_t> seed = {});
			void feedFPS(double);
			void showLoginAndRegisterDialogs(const std::string &hostname);
			bool isConnectedLocally() const;
			bool isConnected() const;
			Ref<Promise<void>> disconnect();
			bool isKeyHeld(int key) const;

			void setGame(std::shared_ptr<ClientGame>);

		private:
			struct KeyInfo {
				int code;
				Modifiers modifiers;
				std::chrono::system_clock::time_point lastProcessed;
			};

			std::shared_ptr<ClientGame> game;
			MTQueue<std::move_only_function<void(Window &)>> functionQueue;
			Lockable<std::list<std::move_only_function<bool(Window &)>>> boolFunctions;
			ServerWrapper serverWrapper;
			std::map<int, KeyInfo> keyTimes;
			std::optional<Vector2i> lastWindowSize;
			std::optional<Vector2i> clickPosition;
			Modifiers lastModifiers;
			std::optional<int> heldMouseButton;
			std::deque<double> fpses;
			double runningSum = 0;
			double runningFPS = 0;
			int fpsCountup = 0;
			bool dragStarted = false;
			bool connectedLocally = false;
			bool connected = false;
			bool mouseMoved = false;

			void keyCallback(int key, int scancode, int action, int mods);
			void charCallback(uint32_t codepoint, int mods);
			void mouseButtonCallback(int button, int action, int mods);
			void mousePositionCallback(int x, int y);
			void scrollCallback(double x_delta, double y_delta);
			void contentScaleCallback(float x_scale, float y_scale);
			void onGameLoaded();
			void autoConnect();
			/** Resolves to false if the user canceled the connection. */
			[[nodiscard]] Ref<Promise<bool>> continueLocalConnection();
			void handleKeys();
			void renderFPSCounter();
			Ref<Promise<void>> newWorld(const std::filesystem::path &, std::optional<size_t> seed);
	};

	using WindowPtr = std::shared_ptr<Window>;
}
