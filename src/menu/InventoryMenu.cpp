#include <nanogui/opengl.h>
#include <nanogui/glutil.h>

#include <GL/glu.h>

#include "game/Game.h"
#include "menu/InventoryMenu.h"
#include "ui/Application.h"
#include "ui/Canvas.h"
#include "util/Util.h"

namespace Game3 {
	InventoryMenu::~InventoryMenu() {
		if (window) {
			window->dispose();
			window = nullptr;
		}
	}

	void InventoryMenu::render(Game &, Canvas &canvas, NVGcontext *) {
		const auto width = canvas.width();
		const auto height = canvas.height();
		constexpr float x_fraction = 4.f;
		constexpr float y_fraction = 6.f;

		const float x = width / x_fraction;
		const float y = height / y_fraction;
		const float w = width * (x_fraction - 2.f) / x_fraction;
		const float h = height * (y_fraction - 2.f) / y_fraction;

		if (window == nullptr) {
			static nanogui::Color light {0.91f, 0.84f, 0.58f, 1.f};
			static nanogui::Color dark  {0.81f, 0.74f, 0.48f, 1.f};
			auto *application = dynamic_cast<Application *>(canvas.parent());
			if (!application)
				throw std::runtime_error("Couldn't cast canvas parent to Application");
			window = new nanogui::Window(application, "Inventory");
			window->performLayout(application->nvgContext());
			window->setPosition({x, y});
			window->setSize({w, h});
			auto *theme = new nanogui::Theme(*window->theme());
			theme->mWindowHeaderGradientTop = light;
			theme->mWindowHeaderGradientBot = dark;
			theme->mWindowTitleFocused = {0.f, 0.f, 0.f, 1.f};
			theme->mWindowFillFocused = light;
			theme->mWindowFillUnfocused = dark;
			theme->mWindowHeaderSepBot = dark;
			window->setTheme(theme);
			window->setModal(true);
			window->requestFocus();
		}
	}
}
