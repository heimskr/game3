#pragma once

#include "game/ClientGame.h"

#include <memory>

#include <nanogui/nanogui.h>

namespace Game3 {
	class Canvas;

	class Application: public nanogui::Screen {
		public:
			std::shared_ptr<ClientGame> game;

			Application();

			~Application() override;

			bool keyboard_event(int key, int scancode, int action, int modifiers) override;
			bool resize_event(const nanogui::Vector2i &) override;
			void draw(NVGcontext *) override;

			static int run(int argc, char **argv);

		private:
			nanogui::Widget *buttonBox = nullptr;
			nanogui::Button *saveButton = nullptr;
			std::unique_ptr<Canvas> canvas;

			void onJoystick(int joystick_id, int event);
			void newGameWindow();
			void newGame(int seed, int width, int height);
			void saveGame();
			void loadGame();

			static Application *instance;
	};

	int runNanogui(int argc, char **argv);
}
