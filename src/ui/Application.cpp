#include <iostream>

#include "ui/Application.h"
#include "ui/Canvas.h"
#include "util/Util.h"

namespace Game3 {
	Application * Application::instance = nullptr;

	Application::Application(): nanogui::Screen(Eigen::Vector2i(1024, 768), "Game3") {
		if (instance != nullptr)
			throw std::runtime_error("Only one Application can exist at a time.");
		instance = this;
		setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill));

		buttonBox = new nanogui::Widget(this);
		buttonBox->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Minimum, -1, -1));

		auto *new_button = new nanogui::Button(buttonBox, "", ENTYPO_ICON_CIRCLE_WITH_PLUS);
		auto *theme = new nanogui::Theme(*new_button->theme());
		theme->mButtonCornerRadius = 0;
		new_button->setTheme(theme);
		new_button->setCallback([this] { newGameWindow(); });

		auto *save_button = new nanogui::Button(buttonBox, "", ENTYPO_ICON_SAVE);
		save_button->setTheme(theme);
		save_button->setEnabled(false);

		auto *open_button = new nanogui::Button(buttonBox, "", ENTYPO_ICON_FOLDER);
		open_button->setTheme(theme);

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
					canvas->center.y() += 0.1f;
					break;
				case GLFW_KEY_UP:
				case GLFW_KEY_W:
					canvas->center.y() -= 0.1f;
					break;
				case GLFW_KEY_LEFT:
				case GLFW_KEY_A:
					canvas->center.x() -= 0.1f;
					break;
				case GLFW_KEY_RIGHT:
				case GLFW_KEY_D:
					canvas->center.x() += 0.1f;
					break;
				default:
					return false;
			}

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

		for (int joystick = GLFW_JOYSTICK_1; joystick <= GLFW_JOYSTICK_LAST; ++joystick)
			if (glfwJoystickPresent(joystick)) {
				int axis_count = 0;
				const float *axes = glfwGetJoystickAxes(joystick, &axis_count);
				if (axes != nullptr && 2 <= axis_count) {
					const float x = axes[0];
					const float divisor = 20.f;
					if (x <= -0.01 || 0.01 <= x)
						canvas->center.x() += x / divisor;
					const float y = axes[1];
					if (y <= -0.01 || 0.01 <= y)
						canvas->center.y() += y / divisor;
				}
			}
	}

	void Application::onJoystick(int joystick_id, int event) {
		std::cout << joystick_id << ": " << event << '\n';
	}

	void Application::newGameWindow() {
		auto *window = new nanogui::Window(this, "New Game");
		window->setLayout(new nanogui::GroupLayout());

		window->add<nanogui::Label>("Seed");
		auto *seedbox = new nanogui::IntBox(window, 1024);
		seedbox->setAlignment(nanogui::TextBox::Alignment::Left);
		seedbox->setEditable(true);

		window->add<nanogui::Label>("Width");
		auto *widthbox = new nanogui::IntBox(window, 256);
		widthbox->setAlignment(nanogui::TextBox::Alignment::Left);
		widthbox->setEditable(true);

		window->add<nanogui::Label>("Height");
		auto *heightbox = new nanogui::IntBox(window, 256);
		heightbox->setAlignment(nanogui::TextBox::Alignment::Left);
		heightbox->setEditable(true);

		auto *panel = new Widget(window);
		panel->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 15));

		(new nanogui::Button(panel, "Create"))->setCallback([=, this] {
			const auto seed = seedbox->value();
			const auto width = widthbox->value();
			const auto height = heightbox->value();
			window->dispose();
			newGame(seed, width, height);
		});

		(new nanogui::Button(panel, "Cancel"))->setCallback([=] {
			window->dispose();
		});

		window->setSize({300, 270});
		window->performLayout(nvgContext());
		window->center();
		window->requestFocus();
	}

	void Application::newGame(int seed, int width, int height) {
		game = std::make_shared<Game>();
		Texture texture("resources/tileset2.png");
		auto tilemap = std::make_shared<Tilemap>(width, height, 16, texture);
		auto realm = std::make_shared<Realm>(1, tilemap);
		realm->generate(seed);
		game->realms.emplace(realm->id, realm);
		game->activeRealm = realm;
		realm->rebind();
		realm->reupload();
		canvas->game = game;
	}
}
