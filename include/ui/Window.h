#pragma once

#include "client/ClientSettings.h"
#include "client/ServerWrapper.h"
#include "data/Identifier.h"
#include "graphics/BatchSpriteRenderer.h"
#include "graphics/CircleRenderer.h"
#include "graphics/GL.h"
#include "graphics/Multiplier.h"
#include "graphics/Overlayer.h"
#include "graphics/Recolor.h"
#include "graphics/Rectangle.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/TextRenderer.h"
#include "threading/Lockable.h"
#include "threading/MTQueue.h"
#include "types/UString.h"
#include "ui/gl/UIContext.h"

#include <GLFW/glfw3.h>

#include <chrono>
#include <functional>
#include <memory>
#include <string>

struct GLFWwindow;

namespace Game3 {
	class Agent;
	class ClientInventory;
	class HasFluids;
	class OmniDialog;
	class Window;
	struct Modifiers;
	struct Position;
	struct RendererContext;

	class Window: public std::enable_shared_from_this<Window> {
		public:
			GLFWwindow *glfwWindow = nullptr;
			std::shared_ptr<ClientGame> game;
			Lockable<ClientSettings> settings;
			std::pair<double, double> center{0, 0};
			double scale{};
			float magic = 8;
			double sizeDivisor = 1.0;
			std::shared_ptr<OmniDialog> omniDialog;
			UIContext uiContext{*this};
			BatchSpriteRenderer  batchSpriteRenderer{*this};
			SingleSpriteRenderer singleSpriteRenderer{*this};
			TextRenderer textRenderer{*this};
			RectangleRenderer rectangleRenderer{*this};
			CircleRenderer circleRenderer{*this};
			Recolor recolor{*this};
			Multiplier multiplier;
			Overlayer overlayer;
			GL::Texture mainTexture;
			GL::Texture staticLightingTexture;
			GL::Texture dynamicLightingTexture;
			GL::FBO fbo;
			Rectangle realmBounds;
			bool autofocus = true;

			Window(GLFWwindow &);

			void queue(std::function<void(Window &)>);
			/* If the given function returns false, it won't be removed from the queue. */
			void queueBool(std::function<bool(Window &)>);
			void delay(std::function<void(Window &)>, uint32_t count = 1);

			int getWidth() const;
			int getHeight() const;
			int getFactor() const;
			int getMouseX() const;
			int getMouseY() const;
			std::pair<double, double> getMouseCoordinates() const;

			const std::shared_ptr<OmniDialog> & getOmniDialog();
			void showOmniDialog();
			void closeOmniDialog();

			void openModule(const Identifier &, const std::any &);
			void removeModule();

			/** Displays an alert. This will reset the dialog pointer. If you need to use this inside a dialog's code, use delay(). */
			void alert(const UString &message, bool queue = true, bool modal = true, bool use_markup = false);

			/** Displays an error message. (See alert.) */
			void error(const UString &message, bool queue = true, bool modal = true, bool use_markup = false);

			Modifiers getModifiers() const;
			Position getHoveredPosition() const;

			void moduleMessageBuffer(const Identifier &module_id, const std::shared_ptr<Agent> &source, const std::string &name, Buffer &&data);

			void activateContext();

			void saveSettings();

			void showExternalInventory(const std::shared_ptr<ClientInventory> &);
			void showFluids(const std::shared_ptr<HasFluids> &);
			GlobalID getExternalGID() const;

			bool inBounds(const Position &) const;
			RendererContext getRendererContext();

			void tick();
			void drawGL();
			void closeGame();

		private:
			struct KeyInfo {
				int code;
				Modifiers modifiers;
				std::chrono::system_clock::time_point lastProcessed;
			};

			MTQueue<std::function<void(Window &)>> functionQueue;
			Lockable<std::list<std::function<bool(Window &)>>> boolFunctions;
			GL::Texture scratchTexture;
			ServerWrapper serverWrapper;
			std::map<int, KeyInfo> keyTimes;

			void keyCallback(int key, int scancode, int action, int mods);
			void onGameLoaded();
			bool connect(const std::string &hostname, uint16_t port);
			void autoConnect();
			void playLocally();
			void continueLocalConnection();
			void handleKeys();
	};
}
