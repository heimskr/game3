#include <iostream>

#include "ui/Application.h"
#include "ui/Canvas.h"

namespace Game3 {
	Application::Application(): nanogui::Screen(Eigen::Vector2i(1024, 768), "Game3") {
		setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill));

		buttonBox = new nanogui::Widget(this);
		buttonBox->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Minimum, -1, -1));

		auto *b = new nanogui::Button(buttonBox, "", ENTYPO_ICON_SAVE);
		auto *theme = new nanogui::Theme(*b->theme());
		theme->mButtonCornerRadius = 0;
		b->setTheme(theme);
		b->setCallback([] { std::cout << ":)\n"; });
		b->setEnabled(false);

		auto *b2 = new nanogui::Button(buttonBox, "", ENTYPO_ICON_FOLDER);
		b2->setTheme(theme);

		canvas = new Canvas(this);

		performLayout();
	}

	bool Application::keyboardEvent(int key, int scancode, int action, int modifiers) {
		if (Screen::keyboardEvent(key, scancode, action, modifiers))
			return true;
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			setVisible(false);
			return true;
		}
		return false;
	}

	bool Application::resizeEvent(const nanogui::Vector2i &new_size) {
		Screen::resizeEvent(new_size);
		canvas->setHeight(new_size.y() - buttonBox->height());
		canvas->setWidth(new_size.x());
		return true;
	}

	void Application::draw(NVGcontext *ctx) {
		Screen::draw(ctx);
	}
}
