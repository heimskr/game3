#include <nanogui/nanogui.h>

#include <fstream>

#include "entity/ItemEntity.h"
#include "game/Game.h"
#include "game/HasInventory.h"
#include "game/Inventory.h"
#include "ui/gtk/NewGameDialog.h"
#include "ui/tab/InventoryTab.h"
#include "ui/tab/TextTab.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"

#include "util/FS.h"
#include "util/Util.h"
#include <GL/glu.h>

// #define USE_CBOR

namespace Game3 {
	static std::chrono::milliseconds arrowTime {100};
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
			if (statusbarWaiting && statusbarExpirationTime <= getTime() - statusbarSetTime) {
				statusbarWaiting = false;
				statusbar.set_text({});
			}
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

		auto click = Gtk::GestureClick::create();
		click->signal_released().connect([this](int n, double x, double y) {
			if (game)
				game->click(n, x, y);
		});
		glArea.add_controller(click);

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

		vbox.append(glArea);
		statusbar.set_halign(Gtk::Align::START);
		statusbar.set_margin(5);
		statusbar.set_margin_start(10);
		vbox.append(statusbar);
		paned.set_orientation(Gtk::Orientation::HORIZONTAL);
		paned.set_start_child(vbox);
		paned.set_end_child(notebook);
		glArea.set_expand(true);
		notebook.set_hexpand(false);
		paned.set_resize_start_child(true);
		paned.set_shrink_start_child(false);
		paned.set_resize_end_child(false);
		paned.set_shrink_end_child(false);
		paned.property_position().signal_changed().connect([this] {
			tabMap.at(notebook.get_nth_page(notebook.get_current_page()))->onResize(game);
		});
		notebook.property_page().signal_changed().connect([this] {
			if (activeTab)
				activeTab->onBlur();
			activeTab = tabMap.at(notebook.get_nth_page(notebook.get_current_page()));
			activeTab->onFocus();
		});

		initTab(inventoryTab, *this).add();
		initTab(textTab, notebook, "", "");
		activeTab = inventoryTab;

		set_child(paned);
		delay([this] {
			paned.set_position(paned.get_width() - 344);
		}, 2);
	}

	void MainWindow::newGame(int seed, int width, int height) {
		glArea.get_context()->make_current();
		game = Game::create(*canvas);
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
		if (!game->player)
			throw std::runtime_error("Player not found in active realm");
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
		glClearColor(.2f, .2f, .2f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		if (autofocus && game && game->player)
			game->player->focus(*canvas, true);
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

	void MainWindow::setStatus(const Glib::ustring &status) {
		statusbar.set_text(status);
		statusbarWaiting = true;
		statusbarSetTime = getTime();
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

	void MainWindow::onKeyReleased(guint keyval, guint keycode, Gdk::ModifierType) {
		keyTimes.erase(keycode);

		if (game && game->player) {
			auto &player = *game->player;
			switch (keyval) {
				case GDK_KEY_w: player.movingUp    = false; break;
				case GDK_KEY_d: player.movingRight = false; break;
				case GDK_KEY_s: player.movingDown  = false; break;
				case GDK_KEY_a: player.movingLeft  = false; break;
			}
		}
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
						player.movingDown = true;
						return;
					case GDK_KEY_w:
						player.movingUp = true;
						return;
					case GDK_KEY_a:
						player.movingLeft = true;
						return;
					case GDK_KEY_d:
						player.movingRight = true;
						return;
					case GDK_KEY_i:
						if (player.inventory->empty()) {
							std::cout << "Inventory empty.\n";
						} else {
							std::cout << "Inventory:\n";
							for (const auto &[slot, stack]: player.inventory->getStorage())
								std::cout << "  " << stack.item->name << " x " << stack.count << " in slot " << slot << '\n';
							inventoryTab->reset(game);
						}
						return;
					case GDK_KEY_o: {
						static std::default_random_engine item_rng;
						static const std::array<ItemID, 3> ids {Item::SHORTSWORD, Item::RED_POTION, Item::COINS};
						ItemStack stack(choose(ids, item_rng), 1);
						auto leftover = player.inventory->add(stack);
						if (leftover) {
							auto &realm = *player.getRealm();
							realm.spawn<ItemEntity>(player.position, *leftover);
							realm.game->signal_player_inventory_update().emit(game->player);
						}
						return;
					}
					case GDK_KEY_r:
						if (game && modifiers == Gdk::ModifierType::CONTROL_MASK)
							game->player->focus(*canvas, false);
						return;
					case GDK_KEY_E:
						game->player->interactOn();
						return;
					case GDK_KEY_e:
						game->player->interactNextTo();
						return;
					case GDK_KEY_u:
						glArea.get_context()->make_current();
						game->activeRealm->reupload();
						return;
					case GDK_KEY_f:
						if (unsigned(modifiers & Gdk::ModifierType::CONTROL_MASK) != 0)
							autofocus = !autofocus;
						else
							game->player->focus(*canvas, false);
						return;
					case GDK_KEY_x:
						setStatus("Hello");
						return;
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
					break;
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

	void MainWindow::onGameLoaded() {
		glArea.get_context()->make_current();
		game->player->focus(*canvas, false);
		canvas->game = game;
		game->activeRealm->rebind();
		game->activeRealm->reupload();
		connectSave();
		for (auto &[widget, tab]: tabMap)
			tab->reset(game);
		for (auto &[id, realm]: game->realms)
			realm->game = game.get();
		game->signal_player_inventory_update().connect([this](const std::shared_ptr<Player> &) {
			inventoryTab->reset(game);
		});
		game->signal_other_inventory_update().connect([this](const std::shared_ptr<HasRealm> &owner) {
			if (auto has_inventory = std::dynamic_pointer_cast<HasInventory>(owner)) {
				if (has_inventory->inventory && has_inventory->inventory == inventoryTab->getExternalInventory())
					inventoryTab->reset(game);
			}
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
