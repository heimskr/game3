#include <nanogui/nanogui.h>

#include <fstream>

#include "entity/ItemEntity.h"
#include "game/Game.h"
#include "ui/gtk/NewGameDialog.h"
#include "ui/tab/InventoryTab.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"

#include "util/FS.h"
#include "util/Util.h"
#include <GL/glu.h>

// #define USE_CBOR

namespace Game3 {
	static std::chrono::milliseconds arrowTime {5};
	static std::chrono::milliseconds interactTime {500};
	std::unordered_map<guint, std::chrono::milliseconds> MainWindow::customKeyRepeatTimes {
		{GDK_KEY_Up,    arrowTime},
		{GDK_KEY_Down,  arrowTime},
		{GDK_KEY_Left,  arrowTime},
		{GDK_KEY_Right, arrowTime},
		{GDK_KEY_e,     interactTime},
		{GDK_KEY_E,     interactTime},
		{GDK_KEY_o,     interactTime},
	};

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

		add_action("open", Gio::ActionMap::ActivateSlot([this] {
			auto *chooser = new Gtk::FileChooserDialog(*this, "Choose File", Gtk::FileChooser::Action::OPEN, true);
			dialog.reset(chooser);
			chooser->set_current_folder(Gio::File::create_for_path(std::filesystem::current_path().string()));
			chooser->set_transient_for(*this);
			chooser->set_modal(true);
			chooser->add_button("_Cancel", Gtk::ResponseType::CANCEL);
			chooser->add_button("_Open", Gtk::ResponseType::OK);
			chooser->signal_response().connect([this, chooser](int response) {
				chooser->hide();
				if (response == Gtk::ResponseType::OK)
					delay([this, chooser] {
						try {
							loadGame(chooser->get_file()->get_path());
						} catch (std::exception &err) {
							error("Error loading save: " + std::string(err.what()));
						}
					});
			});
			chooser->signal_show().connect([this, chooser] {
				chooser->set_default_size(get_width() - 40, get_height() - 40);
				chooser->set_size_request(get_width() - 40, get_height() - 40);
				delay([chooser] {
					if (std::filesystem::exists(Game::DEFAULT_PATH))
						chooser->set_file(Gio::File::create_for_path(Game::DEFAULT_PATH));
				});
			});
			chooser->show();
		}));

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
		glArea.add_controller(drag);

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
		glArea.add_controller(scroll);

		add_tick_callback([this](const auto &) {
			handleKeys();
			return true;
		});

		paned.set_orientation(Gtk::Orientation::HORIZONTAL);
		paned.set_start_child(glArea);
		paned.set_end_child(notebook);
		glArea.set_expand(true);
		notebook.set_hexpand(false);
		paned.set_resize_start_child(true);
		paned.set_shrink_start_child(false);
		paned.set_resize_end_child(false);
		paned.set_shrink_end_child(false);
		notebook.set_size_request(402, -1);
		paned.property_position().signal_changed().connect([this] {
			tabMap.at(notebook.get_nth_page(notebook.get_current_page()))->onResize(game);
		});
		notebook.property_page().signal_changed().connect([this] {
			if (activeTab)
				activeTab->onBlur();
			activeTab = tabMap.at(notebook.get_nth_page(notebook.get_current_page()));
			activeTab->onFocus();
		});

		addTab(inventoryTab = std::make_shared<InventoryTab>(*this));
		activeTab = inventoryTab;

		set_child(paned);
	}

	void MainWindow::newGame(int seed, int width, int height) {
		glArea.get_context()->make_current();
		game = std::make_shared<Game>(*canvas);
		Texture texture("resources/tileset2.png", true);
		texture.init();
		auto tilemap = std::make_shared<Tilemap>(width, height, 16, texture);
		auto realm = Realm::create(1, Realm::OVERWORLD, tilemap);
		realm->game = game.get();
		realm->generate(seed);
		game->realms.emplace(realm->id, realm);
		game->activeRealm = realm;
		realm->add(game->player = Entity::create<Player>(Entity::GANGBLANC));
		game->player->position = {realm->randomLand / width, realm->randomLand % width};
		game->player->init();
		onGameLoaded();
	}

	void MainWindow::loadGame(const std::filesystem::path &path) {
		glArea.get_context()->make_current();
		const std::string data = readFile(path);
		if (!data.empty() && data.front() == '{')
			game = Game::fromJSON(nlohmann::json::parse(data), *canvas);
		else
			game = Game::fromJSON(nlohmann::json::from_cbor(data), *canvas);
		game->initEntities();
		auto realm = game->activeRealm;
		for (const auto &entity: realm->entities)
			if (entity->isPlayer()) {
				if (!(game->player = std::dynamic_pointer_cast<Player>(entity)))
					throw std::runtime_error("Couldn't cast entity with isPlayer() == true to Player");
				break;
			}
		onGameLoaded();
	}

	void MainWindow::saveGame(const std::filesystem::path &path) {
		std::ofstream stream(path);

		if (!stream.is_open()) {
			error("Couldn't open file for writing.");
			return;
		}

#ifdef USE_CBOR
		auto cbor = nlohmann::json::to_cbor(nlohmann::json(*game));
		stream.write(reinterpret_cast<char *>(&cbor[0]), cbor.size());
#else
		stream << nlohmann::json(*game).dump();
#endif
		stream.close();
	}

	bool MainWindow::render(const Glib::RefPtr<Gdk::GLContext> &context) {
		context->make_current();
		glArea.throw_if_error();
		glClearColor(.2f, .2f, .2f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		if (autoFocus && game && game->player)
			game->player->focus(*canvas, false);
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

	Glib::RefPtr<Gdk::GLContext> MainWindow::glContext() {
		return glArea.get_context();
	}

	bool MainWindow::onKeyPressed(guint keyval, guint keycode, Gdk::ModifierType modifiers) {
		if (!keyTimes.contains(keycode)) {
			handleKey(keyval, keycode, modifiers);
			if (unsigned(modifiers & Gdk::ModifierType::CONTROL_MASK) == 0)
				keyTimes.try_emplace(keycode, keyval, modifiers, getTime());
		} else
			keyTimes.at(keycode).modifiers = modifiers;
		return true;
	}

	void MainWindow::onKeyReleased(guint, guint keycode, Gdk::ModifierType) {
		keyTimes.erase(keycode);
	}

	void MainWindow::handleKeys() {
		for (auto &[keycode, info]: keyTimes) {
			auto &[keyval, modifiers, time] = info;
			auto repeat_time = keyRepeatTime;
			if (customKeyRepeatTimes.contains(keyval))
				repeat_time = customKeyRepeatTimes.at(keyval);
			if (std::chrono::duration_cast<std::chrono::milliseconds>(timeDifference(time)) < repeat_time)
				continue;
			time = getTime();
			handleKey(keyval, keycode, modifiers);
		}
	}

	void MainWindow::handleKey(guint keyval, guint, Gdk::ModifierType modifiers) {
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
							inventoryTab->reset(game);
						}
						return;
					case GDK_KEY_o: {
						ItemStack sword(Item::SHORTSWORD, 1);
						auto leftover = player.inventory.add(sword);
						if (leftover) {
							auto &realm = *player.getRealm();
							realm.spawn<ItemEntity>(player.position, *leftover);
							realm.game->signal_player_inventory_update().emit(game->player);
						}
						return;
					}
					case GDK_KEY_r:
						if (game && modifiers == Gdk::ModifierType::CONTROL_MASK)
							game->player->focus(*canvas);
						break;
					case GDK_KEY_E:
						game->player->interactOn();
						break;
					case GDK_KEY_e:
						game->player->interactNextTo();
						break;
					case GDK_KEY_u:
						glArea.get_context()->make_current();
						game->activeRealm->reupload();
						break;
					case GDK_KEY_f:
						if (unsigned(modifiers & Gdk::ModifierType::CONTROL_MASK) != 0)
							autoFocus = !autoFocus;
						else
							game->player->focus(*canvas);
						break;
				}
			}

			const float delta = canvas->scale / 20.f;
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
		auto *new_dialog = new NewGameDialog(*this);
		dialog.reset(new_dialog);
		new_dialog->set_transient_for(*this);
		new_dialog->signal_submit().connect(sigc::mem_fun(*this, &MainWindow::newGame));
		new_dialog->show();
	}

	void MainWindow::connectSave() {
		add_action("save", Gio::ActionMap::ActivateSlot([this] {
			auto *chooser = new Gtk::FileChooserDialog(*this, "Save Location", Gtk::FileChooser::Action::SAVE, true);
			dialog.reset(chooser);
			chooser->set_current_folder(Gio::File::create_for_path(std::filesystem::current_path().string()));
			chooser->set_transient_for(*this);
			chooser->set_modal(true);
			chooser->add_button("_Cancel", Gtk::ResponseType::CANCEL);
			chooser->add_button("_Save", Gtk::ResponseType::OK);
			chooser->signal_response().connect([this, chooser](int response) {
				chooser->hide();
				if (response == Gtk::ResponseType::OK)
					delay([this, chooser] {
						try {
							saveGame(chooser->get_file()->get_path());
						} catch (std::exception &err) {
							error("Error loading save: " + std::string(err.what()));
						}
					});
			});
			chooser->signal_show().connect([this, chooser] {
				chooser->set_default_size(get_width() - 40, get_height() - 40);
				chooser->set_size_request(get_width() - 40, get_height() - 40);
			});
			chooser->show();
		}));
	}

	void MainWindow::addTab(std::shared_ptr<Tab> tab) {
		tabMap.emplace(&tab->getWidget(), tab);
		notebook.append_page(tab->getWidget(), tab->getName());
	}

	void MainWindow::onGameLoaded() {
		glArea.get_context()->make_current();
		game->player->focus(*canvas);
		canvas->game = game;
		game->activeRealm->rebind();
		game->activeRealm->reupload();
		connectSave();
		for (auto &[widget, tab]: tabMap)
			tab->reset(game);
		for (auto &[id, realm]: game->realms)
			realm->game = game.get();
		game->signal_player_inventory_update().connect([this](const auto &) {
			inventoryTab->reset(game);
		});
	}

/*
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
