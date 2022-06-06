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

		add_action("new", Gio::ActionMap::ActivateSlot(sigc::mem_fun(*this, &MainWindow::onNew)));

		glArea.set_expand(true);
		glArea.set_required_version(3, 3);
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
		glArea.set_focusable(true);
		set_child(glArea);

		auto key_controller = Gtk::EventControllerKey::create();
		key_controller->signal_key_pressed().connect(sigc::mem_fun(*this, &MainWindow::onKeyPressed), false);
		key_controller->signal_key_released().connect(sigc::mem_fun(*this, &MainWindow::onKeyReleased));
		add_controller(key_controller);

		auto drag = Gtk::GestureDrag::create();
		drag->signal_drag_begin().connect([this](double, double) {
			lastDragX = lastDragY = 0;
		});
		drag->signal_drag_update().connect([this](double x, double y) {
			double delta_x = x - lastDragX;
			double delta_y = y - lastDragY;
			lastDragX = x;
			lastDragY = y;
			if (canvas) {
				canvas->center.x() += delta_x / (canvas->magic * canvas->scale);
				canvas->center.y() += delta_y / (canvas->magic * canvas->scale);
			}
		});
		add_controller(drag);

		auto motion = Gtk::EventControllerMotion::create();
		motion->signal_motion().connect([this](double x, double y) {
			glAreaMouseX = x;
			glAreaMouseY = y;
		});
		glArea.add_controller(motion);

		auto scroll = Gtk::EventControllerScroll::create();
		scroll->set_flags(Gtk::EventControllerScroll::Flags::VERTICAL);
		scroll->signal_scroll().connect([this](double, double y) {
			if (!canvas)
				return true;

			const auto old_scale = canvas->scale;

			if (y == -1)
				canvas->scale *= 1.08f;
			else if (y == 1)
				canvas->scale /= 1.08f;

			const auto w = glArea.get_width();
			const auto h = glArea.get_height();

			const auto difference_x = w / old_scale - w / canvas->scale;
			const auto side_ratio_x = (glAreaMouseX - w / 2.f) / w;
			canvas->center.x() -= difference_x * side_ratio_x / 8.f;

			const auto difference_y = h / old_scale - h / canvas->scale;
			const auto side_ratio_y = (glAreaMouseY - h / 2.f) / h;
			canvas->center.y() -= difference_y * side_ratio_y / 8.f;

			return true;
		}, false);
		add_controller(scroll);

		add_tick_callback([this](const auto &) {
			handleKeys();
			return true;
		});
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
		glClearColor(.2f, .2f, .2f, 1.f); CHECKGL
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

	bool MainWindow::onKeyPressed(guint keyval, guint keycode, Gdk::ModifierType modifiers) {
		if (!keyTimes.contains(keyval)) {
			handleKey(keyval, keycode, modifiers);
			keyTimes.try_emplace(keyval, keycode, modifiers, getTime());
		} else
			keyTimes.at(keyval).modifiers = modifiers;
		return true;
	}

	void MainWindow::onKeyReleased(guint keyval, guint, Gdk::ModifierType) {
		keyTimes.erase(keyval);
	}

	void MainWindow::handleKeys() {
		for (auto &[keyval, info]: keyTimes) {
			auto &[keycode, modifiers, time] = info;
			if (std::chrono::duration_cast<std::chrono::milliseconds>(timeDifference(time)) < keyRepeatTime)
				continue;
			time = getTime();
			handleKey(keyval, keycode, modifiers);
		}
	}

	void MainWindow::handleKey(guint keyval, guint, Gdk::ModifierType) {
		if (canvas) {
			if (game && game->player) {
				auto &player = *game->player;
				switch (keyval) {
					case GDK_KEY_s:
						player.move(Direction::Down);
						return;
					case GDK_KEY_w:
						player.move(Direction::Up);
						return;
					case GDK_KEY_a:
						player.move(Direction::Left);
						return;
					case GDK_KEY_d:
						player.move(Direction::Right);
						return;
					case GDK_KEY_i:
						if (player.inventory.empty()) {
							std::cout << "Inventory empty.\n";
						} else {
							std::cout << "Inventory:\n";
							for (const auto &[slot, stack]: player.inventory.getStorage())
								std::cout << "  " << stack.item->name << " x " << stack.count << " in slot " << slot << '\n';
						}
						return;
					case GDK_KEY_o: {
						ItemStack sword(Item::SHORTSWORD, 1);
						auto leftover = player.inventory.add(sword);
						std::cout << "Added sword. ";
						if (leftover)
							std::cout << "Left over: " << leftover->item->name << " x " << leftover->count << '\n';
						else
							std::cout << "No leftover.\n";
						return;
					}
				}
			}

			const float delta = canvas->scale / 4.f;
			switch (keyval) {
				case GDK_KEY_Down:
					canvas->center.y() -= delta;
					break;
				case GDK_KEY_Up:
					canvas->center.y() += delta;
					break;
				case GDK_KEY_Left:
					canvas->center.x() += delta;
					break;
				case GDK_KEY_Right:
					canvas->center.x() -= delta;
					break;
				default:
					return;
			}
		}
	}

	void MainWindow::onNew() {
		newGame(666, 256, 256);
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
*/
}
