#include <iostream>

#include "ui/Application.h"
#include "ui/Canvas.h"

namespace Game3 {
	Application * Application::instance = nullptr;

	Application::Application(): nanogui::Screen(Eigen::Vector2i(1024, 768), "Game3") {
		if (instance != nullptr)
			throw std::runtime_error("Only one Application can exist at a time.");
		instance = this;
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

		glfwSetJoystickCallback(+[](int joystick_id, int event) {
			std::cout << joystick_id << ": " << event << '\n';
			if (Application::instance)
				Application::instance->onJoystick(joystick_id, event);
		});

		canvas = new Canvas(this);

		performLayout();
	}

	Application::~Application() {
		instance = nullptr;
	}

	bool Application::keyboardEvent(int key, int scancode, int action, int modifiers) {
		if (Screen::keyboardEvent(key, scancode, action, modifiers))
			return true;

		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			setVisible(false);
			return true;
		}

		if (canvas) {
			std::cout << key << ", " << scancode << ", " << action << ", " << modifiers << '\n';
			switch (key) {
				case GLFW_KEY_DOWN:
				case GLFW_KEY_S:
					canvas->center().y() += 0.1f;
					return true;
				case GLFW_KEY_UP:
				case GLFW_KEY_W:
					canvas->center().y() -= 0.1f;
					return true;
				case GLFW_KEY_LEFT:
				case GLFW_KEY_A:
					canvas->center().x() -= 0.1f;
					return true;
				case GLFW_KEY_RIGHT:
				case GLFW_KEY_D:
					canvas->center().x() += 0.1f;
					return true;
			}
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

		for (int joystick = GLFW_JOYSTICK_1; joystick <= GLFW_JOYSTICK_LAST; ++joystick)
			if (glfwJoystickPresent(joystick)) {
				int axis_count = 0;
				const float *axes = glfwGetJoystickAxes(joystick, &axis_count);
				if (axes != nullptr && 2 <= axis_count) {
					const float x = axes[0];
					const float divisor = 20.f;
					if (x <= -0.01 || 0.01 <= x)
						canvas->center().x() += x / divisor;
					const float y = axes[1];
					if (y <= -0.01 || 0.01 <= y)
						canvas->center().y() += y / divisor;
				}
			}
	}

	void Application::onJoystick(int joystick_id, int event) {
		std::cout << joystick_id << ": " << event << '\n';
	}
}
