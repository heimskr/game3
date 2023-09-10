#include <fstream>

#include "ui/Canvas.h"
#include "ui/Nanogui.h"
#include "util/FS.h"

#include <nanogui/icons.h>

namespace Game3 {
	Application * Application::instance = nullptr;

	Application::Application(): nanogui::Screen(nanogui::Vector2i(1024 * 2, 768 * 2), "Game3", true, false, true, true, false, 4, 1) {
		if (instance != nullptr)
			throw std::runtime_error("Only one Application can exist at a time.");
		instance = this;
		set_layout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill));

		buttonBox = new nanogui::Widget(this);
		buttonBox->set_layout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Minimum, -1, -1));

		auto *new_button = new nanogui::Button(buttonBox, "", FA_PLUS_CIRCLE);
		auto *theme = new nanogui::Theme(*new_button->theme());
		theme->m_button_corner_radius = 0;
		new_button->set_theme(theme);
		new_button->set_callback([this] { newGameWindow(); });

		saveButton = new nanogui::Button(buttonBox, "", FA_SAVE);
		saveButton->set_theme(theme);
		saveButton->set_callback([this] { saveGame(); });
		saveButton->set_enabled(false);

		auto *open_button = new nanogui::Button(buttonBox, "", FA_FOLDER);
		open_button->set_theme(theme);
		open_button->set_callback([this] { loadGame(); });

		auto *reset_button = new nanogui::Button(buttonBox, "", FA_SEARCH);
		reset_button->set_theme(theme);
		reset_button->set_callback([this] {
			canvas->scale = Canvas::DEFAULT_SCALE;
			canvas->center = {0.f, 0.f};
		});

		glfwSetJoystickCallback(+[](int joystick_id, int event) {
			std::cout << joystick_id << ": " << event << '\n';
			if (Application::instance)
				Application::instance->onJoystick(joystick_id, event);
		});

		canvas = std::make_unique<Canvas>(*this);

		perform_layout();
	}

	Application::~Application() {
		instance = nullptr;
	}

	bool Application::keyboard_event(int key, int scancode, int action, int modifiers) {
		if (Screen::keyboard_event(key, scancode, action, modifiers))
			return true;

		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			set_visible(false);
			return true;
		}

		if (canvas) {
			// std::cout << key << ", " << scancode << ", " << action << ", " << modifiers << '\n';
			// if (game && game->player && action != 0) {
			// 	auto &player = *game->player;
			// 	switch (key) {
			// 		case GLFW_KEY_S:
			// 			player.move(Direction::Down);
			// 			return true;
			// 		case GLFW_KEY_W:
			// 			player.move(Direction::Up);
			// 			return true;
			// 		case GLFW_KEY_A:
			// 			player.move(Direction::Left);
			// 			return true;
			// 		case GLFW_KEY_D:
			// 			player.move(Direction::Right);
			// 			return true;
			// 		case GLFW_KEY_I: {
			// 			if (game->menu && game->menu->getType() == MenuType::Inventory) {
			// 				game->menu.reset();
			// 			} else {
			// 				auto menu = std::make_shared<InventoryMenu>(game->player);
			// 				game->menu = menu;
			// 			}
			// 			// if (player.inventory.empty()) {
			// 			// 	std::cout << "Inventory empty.\n";
			// 			// } else {
			// 			// 	std::cout << "Inventory:\n";
			// 			// 	for (const auto &[slot, stack]: player.inventory.getStorage())
			// 			// 		std::cout << "  " << stack.item->name << " x " << stack.count << " in slot " << slot << '\n';
			// 			// }

			// 			return true;
			// 		}
			// 		case GLFW_KEY_O: {
			// 			ItemStack sword(Item::SHORTSWORD, 1);
			// 			auto leftover = player.inventory.add(sword);
			// 			std::cout << "Added sword. ";
			// 			if (leftover)
			// 				std::cout << "Left over: " << leftover->item->name << " x " << leftover->count << '\n';
			// 			else
			// 				std::cout << "No leftover.\n";
			// 		}
			// 	}
			// }

			const float delta = canvas->scale / 4.f;
			switch (key) {
				case GLFW_KEY_DOWN:
					canvas->center.y() -= delta;
					break;
				case GLFW_KEY_UP:
					canvas->center.y() += delta;
					break;
				case GLFW_KEY_LEFT:
					canvas->center.x() += delta;
					break;
				case GLFW_KEY_RIGHT:
					canvas->center.x() -= delta;
					break;
				default:
					return false;
			}

			return true;
		}

		return false;
	}

	bool Application::resize_event(const nanogui::Vector2i &new_size) {
		Screen::resize_event(new_size);
		canvas->set_height(new_size.y() - buttonBox->height());
		canvas->set_width(new_size.x());
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
		window->set_layout(new nanogui::GroupLayout());

		window->add<nanogui::Label>("Seed");
		auto *seedbox = new nanogui::IntBox(window, 666);
		seedbox->set_alignment(nanogui::TextBox::Alignment::Left);
		seedbox->set_editable(true);

		window->add<nanogui::Label>("Width");
		auto *widthbox = new nanogui::IntBox(window, 256);
		widthbox->set_alignment(nanogui::TextBox::Alignment::Left);
		widthbox->set_editable(true);

		window->add<nanogui::Label>("Height");
		auto *heightbox = new nanogui::IntBox(window, 256);
		heightbox->set_alignment(nanogui::TextBox::Alignment::Left);
		heightbox->set_editable(true);

		auto *panel = new Widget(window);
		panel->set_layout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 15));

		(new nanogui::Button(panel, "Create"))->set_callback([=, this] {
			const auto seed = seedbox->value();
			const auto width = widthbox->value();
			const auto height = heightbox->value();
			window->dispose();
			newGame(seed, width, height);
		});

		(new nanogui::Button(panel, "Cancel"))->set_callback([=] {
			window->dispose();
		});

		window->set_size({300, 270});
		window->perform_layout(nvg_context());
		window->center();
		window->request_focus();
	}

	void Application::newGame(int seed, int width, int height) {
		// game = std::make_shared<Game>();
		// Texture texture("resources/tileset2.png");
		// texture.init();
		// auto tilemap = std::make_shared<Tilemap>(width, height, 16, texture);
		// auto realm = std::make_shared<Realm>(1, tilemap);
		// realm->generate(seed);
		// game->realms.emplace(realm->id, realm);
		// game->activeRealm = realm;
		// realm->rebind();
		// realm->reupload();
		// realm->addEntity(game->player = std::make_shared<Player>(Entity::GANGBLANC));
		// game->player->position = {realm->randomLand / width, realm->randomLand % width};
		// game->player->init();
		// game->player->focus(*canvas);
		// canvas->game = game;
		// saveButton->setEnabled(true);
	}

	void Application::saveGame() {
		if (!game)
			return;

		const std::string path = nanogui::file_dialog({{"g3", "Game3 save"}}, true);

		if (path.empty())
			return;

		std::ofstream stream(path);

		if (!stream.is_open()) {
			new nanogui::MessageDialog(this, nanogui::MessageDialog::Type::Warning, "Error", "Couldn't open file for writing.");
			return;
		}

#ifdef USE_CBOR
		auto cbor = nlohmann::json::to_cbor(nlohmann::json(*game));
		stream.write(reinterpret_cast<char *>(&cbor[0]), cbor.size());
#else
		stream << nlohmann::json(*game).dump();
#endif
		stream.close();

		new nanogui::MessageDialog(this, nanogui::MessageDialog::Type::Information, "Success", "Game saved.");
	}

	void Application::loadGame() {
		const std::string path = nanogui::file_dialog({{"g3", "Game3 save"}}, false);

		if (path.empty())
			return;

		// const std::string data = readFile(path);
		// if (!data.empty() && data.front() == '{')
		// 	game = std::make_shared<Game>(nlohmann::json::parse(data));
		// else
		// 	game = std::make_shared<Game>(nlohmann::json::from_cbor(data));
		// game->initEntities();
		// auto realm = game->activeRealm;
		// for (const auto &entity: realm->entities)
		// 	if (entity->isPlayer()) {
		// 		if (!(game->player = std::dynamic_pointer_cast<Player>(entity)))
		// 			throw std::runtime_error("Couldn't cast entity with isPlayer() == true to Player");
		// 		break;
		// 	}
		// // realm->rebind();
		// realm->reupload();
		// // game->player->focus(*canvas);
		// canvas->game = game;
		saveButton->set_enabled(true);
	}

	int Application::run(int argc, char **argv) {
		nanogui::init();
		{
			nanogui::ref<Application> app = new Application;
			app->draw_all();
			app->set_visible(true);
			nanogui::mainloop(1000 / 144);
		}
		nanogui::shutdown();
		return 0;
	}
}