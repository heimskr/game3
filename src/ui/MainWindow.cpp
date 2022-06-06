#include <nanogui/nanogui.h>

#include "game/Game.h"
#include "ui/MainWindow.h"

#include "util/Util.h"
#include <GL/glu.h>

namespace Game3 {
	MainWindow::MainWindow(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder_):
	Gtk::ApplicationWindow(cobject), builder(builder_) {
		header = builder->get_widget<Gtk::HeaderBar>("headerbar");
		set_titlebar(*header);

		cssProvider = Gtk::CssProvider::create();
		cssProvider->load_from_resource("/game3/style.css");
		Gtk::StyleContext::add_provider_for_display(Gdk::Display::get_default(), cssProvider,
			GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

		functionQueueDispatcher.connect([this] {
			auto lock = std::unique_lock(functionQueueMutex);
			for (auto fn: functionQueue)
				fn();
			functionQueue.clear();
		});

		add_action("example", Gio::ActionMap::ActivateSlot([this] {
			if (!canvas)
				return;
			newGame(666, 256, 256);
		}));

		glArea.set_expand(true);
		glArea.set_required_version(3, 3);
		// glArea.set_has_depth_buffer(true);
		glArea.set_has_stencil_buffer(true);
		glArea.signal_realize().connect([this] {
			glArea.make_current();
			glArea.throw_if_error();
			canvas = std::make_unique<Canvas>(*this);
		});
		glArea.signal_unrealize().connect([this] {
			glArea.make_current();
			glArea.throw_if_error();
		});
		glArea.signal_render().connect(sigc::mem_fun(*this, &MainWindow::render), false);
		glArea.set_auto_render(false);
		glArea.add_tick_callback([this](const Glib::RefPtr<Gdk::FrameClock> &) {
			glArea.queue_render();
			return true;
		});
		set_child(glArea);
	}

	void MainWindow::newGame(int seed, int width, int height) {
		game = std::make_shared<Game>();
		glArea.get_context()->make_current();
		Texture texture("resources/tileset2.png", true);
		texture.init();
		auto tilemap = std::make_shared<Tilemap>(width, height, 16, texture);
		auto realm = std::make_shared<Realm>(1, tilemap);
		realm->generate(seed);
		game->realms.emplace(realm->id, realm);
		game->activeRealm = realm;
		realm->addEntity(game->player = std::make_shared<Player>(Entity::GANGBLANC));
		game->player->position = {realm->randomLand / width, realm->randomLand % width};
		game->player->init();
		game->player->focus(*canvas);
		canvas->game = game;
		realm->rebind();
		realm->reupload();
	}

	bool MainWindow::render(const Glib::RefPtr<Gdk::GLContext> &context) {
		context->make_current();
		glArea.throw_if_error();
		glClearColor(0.2, 0.2, 0.2, 1.0); CHECKGL
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); CHECKGL
		canvas->drawGL();
		return true;
	}

	MainWindow * MainWindow::create() {
		auto builder = Gtk::Builder::create_from_resource("/game3/window.ui");
		auto window = Gtk::Builder::get_widget_derived<MainWindow>(builder, "main_window");
		if (!window)
			throw std::runtime_error("No \"main_window\" object in window.ui");
		return window;
	}

	void MainWindow::delay(std::function<void()> fn, unsigned count) {
		if (count <= 1)
			add_tick_callback([fn](const auto &) {
				fn();
				return false;
			});
		else
			delay([this, fn, count] {
				delay(fn, count - 1);
			});
	}

	void MainWindow::queue(std::function<void()> fn) {
		{
			auto lock = std::unique_lock(functionQueueMutex);
			functionQueue.push_back(fn);
		}
		functionQueueDispatcher.emit();
	}

	void MainWindow::alert(const Glib::ustring &message, Gtk::MessageType type, bool modal, bool use_markup) {
		dialog.reset(new Gtk::MessageDialog(*this, message, use_markup, type, Gtk::ButtonsType::OK, modal));
		dialog->signal_response().connect([this](int) {
			dialog->close();
		});
		dialog->show();
	}

	void MainWindow::error(const Glib::ustring &message, bool modal, bool use_markup) {
		alert(message, Gtk::MessageType::ERROR, modal, use_markup);
	}

/*
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

		const std::string data = readFile(path);
		if (!data.empty() && data.front() == '{')
			game = std::make_shared<Game>(nlohmann::json::parse(data));
		else
			game = std::make_shared<Game>(nlohmann::json::from_cbor(data));
		game->initEntities();
		auto realm = game->activeRealm;
		for (const auto &entity: realm->entities)
			if (entity->isPlayer()) {
				if (!(game->player = std::dynamic_pointer_cast<Player>(entity)))
					throw std::runtime_error("Couldn't cast entity with isPlayer() == true to Player");
				break;
			}
		realm->rebind();
		realm->reupload();
		game->player->focus(*canvas);
		canvas->game = game;
		saveButton->setEnabled(true);
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

	bool Application::keyboardEvent(int key, int scancode, int action, int modifiers) {
		if (Screen::keyboardEvent(key, scancode, action, modifiers))
			return true;

		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			setVisible(false);
			return true;
		}

		if (canvas) {
			// std::cout << key << ", " << scancode << ", " << action << ", " << modifiers << '\n';
			if (game && game->player && action != 0) {
				auto &player = *game->player;
				switch (key) {
					case GLFW_KEY_S:
						player.move(Direction::Down);
						return true;
					case GLFW_KEY_W:
						player.move(Direction::Up);
						return true;
					case GLFW_KEY_A:
						player.move(Direction::Left);
						return true;
					case GLFW_KEY_D:
						player.move(Direction::Right);
						return true;
					case GLFW_KEY_I: {
						if (game->menu && game->menu->getType() == MenuType::Inventory) {
							game->menu.reset();
						} else {
							auto menu = std::make_shared<InventoryMenu>(game->player);
							game->menu = menu;
						}
						// if (player.inventory.empty()) {
						// 	std::cout << "Inventory empty.\n";
						// } else {
						// 	std::cout << "Inventory:\n";
						// 	for (const auto &[slot, stack]: player.inventory.getStorage())
						// 		std::cout << "  " << stack.item->name << " x " << stack.count << " in slot " << slot << '\n';
						// }

						return true;
					}
					case GLFW_KEY_O: {
						ItemStack sword(Item::SHORTSWORD, 1);
						auto leftover = player.inventory.add(sword);
						std::cout << "Added sword. ";
						if (leftover)
							std::cout << "Left over: " << leftover->item->name << " x " << leftover->count << '\n';
						else
							std::cout << "No leftover.\n";
					}
				}
			}

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
*/
}
