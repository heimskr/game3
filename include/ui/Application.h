#pragma once

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/colorpicker.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <stb_image.h>

#include "game/Game.h"
#include "lib/GLTexture.h"
#include "ui/Canvas.h"

namespace Game3 {
	class Application: public nanogui::Screen {
		public:
			std::shared_ptr<Game> game;

			Application();

			~Application();

			bool keyboardEvent(int key, int scancode, int action, int modifiers) override;
			bool resizeEvent(const nanogui::Vector2i &) override;
			void draw(NVGcontext *) override;

		private:
			nanogui::Widget *buttonBox = nullptr;
			Canvas *canvas = nullptr;

			void onJoystick(int joystick_id, int event);
			void newGameWindow();
			void newGame(int seed, int width, int height);

			static Application *instance;
	};
}
