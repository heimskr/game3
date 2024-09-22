#pragma once

#include "client/ClientSettings.h"
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

#include <functional>
#include <memory>

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

	class Window {
		public:
			GLFWwindow *glfwWindow = nullptr;
			std::shared_ptr<ClientGame> game;
			Lockable<ClientSettings> settings;
			std::pair<double, double> center{0, 0};
			double scale{};
			float magic = 8;
			double sizeDivisor = 1.0;
			MTQueue<std::function<void(Window &)>> functionQueue;
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

			Window(GLFWwindow &);

			void queue(std::function<void(Window &)>);

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

			void drawGL();

		private:
			GL::Texture scratchTexture;
	};
}
