#include <deque>
#include <fstream>
#include <iostream>

#include "Shader.h"
#include "Tileset.h"

#include "entity/ItemEntity.h"
#include "entity/Merchant.h"
#include "entity/Miner.h"
#include "game/Game.h"
#include "game/HasInventory.h"
#include "game/Inventory.h"
#include "realm/Overworld.h"
#include "tileentity/Building.h"
#include "tileentity/Teleporter.h"
#include "ui/gtk/CommandDialog.h"
#include "ui/gtk/NewGameDialog.h"
#include "ui/tab/CraftingTab.h"
#include "ui/tab/InventoryTab.h"
#include "ui/tab/MerchantTab.h"
#include "ui/tab/TextTab.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "util/FS.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Overworld.h"
#include "worldgen/WorldGen.h"

// #define USE_CBOR

namespace Game3 {
	static std::chrono::milliseconds arrowTime {100};
	static std::chrono::milliseconds interactTime {500};
	static std::chrono::milliseconds slowTime {1000};
	std::unordered_map<guint, std::chrono::milliseconds> MainWindow::customKeyRepeatTimes {
		{GDK_KEY_Up,    arrowTime},
		{GDK_KEY_Down,  arrowTime},
		{GDK_KEY_Left,  arrowTime},
		{GDK_KEY_Right, arrowTime},
		{GDK_KEY_e,     interactTime},
		{GDK_KEY_E,     interactTime},
		{GDK_KEY_o,     interactTime},
		{GDK_KEY_space, interactTime},
		{GDK_KEY_g,     slowTime},
		{GDK_KEY_r,     slowTime},
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

		debugAction = add_action_bool("debug", [this] {
			if (game) {
				game->debugMode = !game->debugMode;
				debugAction->set_state(Glib::Variant<bool>::create(game->debugMode));
			}
		}, false);

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
		glArea.add_tick_callback([this](const Glib::RefPtr<Gdk::FrameClock> &clock) {
			glArea.queue_render();
			if (game) {
				if (autofocus && game->player)
					game->player->focus(*canvas, true);
				const int minute = game->getMinute();
				timeLabel.set_text(std::to_string(int(game->getHour())) + (minute < 10? ":0" : ":") + std::to_string(minute));
			}
			static std::deque<double> fpses;
			fpses.push_back(clock->get_fps());
			if (36 < fpses.size())
				fpses.pop_front();
			double sum = 0;
			for (const double fps: fpses)
				sum += fps;
			statusbar.set_text(std::to_string(int(sum / fpses.size())) + " FPS");
			if (statusbarWaiting && statusbarExpirationTime <= getTime() - statusbarSetTime) {
				statusbarWaiting = false;
				statusbar.set_text({});
			}
			return true;
		});
		glArea.set_focusable(true);

		glMenu.set_parent(glArea);

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

		auto left_click  = Gtk::GestureClick::create();
		auto right_click = Gtk::GestureClick::create();
		left_click->set_button(1);
		right_click->set_button(3);
		left_click->signal_released().connect([this](int n, double x, double y) {
			glArea.grab_focus();
			if (game)
				game->click(1, n, x, y);
		});
		right_click->signal_released().connect([this](int n, double x, double y) {
			glArea.grab_focus();
			if (game)
				game->click(3, n, x, y);
		});
		glArea.add_controller(left_click);
		glArea.add_controller(right_click);

		auto forward  = Gtk::GestureClick::create();
		auto backward = Gtk::GestureClick::create();
		forward->set_button(9);
		backward->set_button(8);
		forward->signal_pressed().connect([this](int, double, double) {
			if (game && game->player) {
				game->player->inventory->nextSlot();
				inventoryTab->reset(game);
			}
		});
		backward->signal_pressed().connect([this](int, double, double) {
			if (game && game->player) {
				game->player->inventory->prevSlot();
				inventoryTab->reset(game);
			}
		});
		glArea.add_controller(forward);
		glArea.add_controller(backward);

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
		statusbar.set_hexpand(true);
		statusBox.append(statusbar);
		timeLabel.set_margin_start(10);
		timeLabel.set_margin_end(5);
		statusBox.append(timeLabel);
		vbox.append(statusBox);
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
		initTab(craftingTab, *this).add();
		initTab(textTab, notebook, "", "");
		initTab(merchantTab, *this);
		activeTab = inventoryTab;

		set_child(paned);
		delay([this] {
			paned.set_position(paned.get_width() - 344);
		}, 2);
	}

	void MainWindow::newGame(size_t seed, const WorldGenParams &params) {
		Timer timer("NewGame");
		glArea.get_context()->make_current();
		game = Game::create(*canvas);
		game->initEntities();
		auto realm = Realm::create<Overworld>(*game, 1, Overworld::ID(), "base:tileset/monomap"_id, seed);
		realm->outdoors = true;
		std::default_random_engine rng;
		rng.seed(seed);
		WorldGen::generateOverworld(realm, seed, params, {{-1, -1}, {1, 1}}, true);
		game->realms.emplace(realm->id, realm);
		game->activeRealm = realm;
		realm->add(game->player = Entity::create<Player>());
		game->player->position = realm->randomLand;
		game->player->init(*game);
		onGameLoaded();
		game->player->inventory->add(ItemStack::withDurability(*game, "base:item/iron_pickaxe"));
		game->player->inventory->add(ItemStack::withDurability(*game, "base:item/iron_shovel"));
		game->player->inventory->add(ItemStack::withDurability(*game, "base:item/iron_axe"));
		game->player->inventory->add(ItemStack::withDurability(*game, "base:item/iron_hammer"));
		game->player->inventory->add(ItemStack(*game, "base:item/cave_entrance", 4));
		timer.stop();
		Timer::summary();
		Timer::clear();
	}

	void MainWindow::loadGame(const std::filesystem::path &path) {
		glArea.get_context()->make_current();
		const std::string data = readFile(path);
		if (!data.empty() && data.front() == '{')
			game = Game::fromJSON(nlohmann::json::parse(data), *canvas);
		else
			game = Game::fromJSON(nlohmann::json::from_cbor(data), *canvas);
		game->initEntities();
		// for (auto &[id, realm]: game->realms)
		// 	realm->remakePathMap();
		for (const auto &entity: game->activeRealm->entities)
			if (entity->isPlayer()) {
				if (!(game->player = std::dynamic_pointer_cast<Player>(entity)))
					throw std::runtime_error("Couldn't cast entity with isPlayer() == true to Player");
				break;
			}
		if (!game->player)
			throw std::runtime_error("Player not found in active realm");
		for (const auto &[id, realm]: game->realms)
			for (const auto &entity: realm->entities)
				entity->initAfterLoad(*game);
		onGameLoaded();
	}

	void MainWindow::onGameLoaded() {
		glArea.get_context()->make_current();
		debugAction->set_state(Glib::Variant<bool>::create(game->debugMode));
		game->player->focus(*canvas, false);
		game->initInteractionSets();
		canvas->game = game;
		game->activeRealm->reupload();
		connectSave();
		for (auto &[widget, tab]: tabMap)
			tab->reset(game);
		game->signal_player_inventory_update().connect([this](const PlayerPtr &) {
			inventoryTab->reset(game);
			if (isFocused(merchantTab))
				merchantTab->reset(game);
			craftingTab->reset(game);
		});
		game->signal_other_inventory_update().connect([this](const std::shared_ptr<HasRealm> &owner) {
			if (auto has_inventory = std::dynamic_pointer_cast<HasInventory>(owner)) {
				if (has_inventory->inventory) {
					if (has_inventory->inventory == inventoryTab->getExternalInventory())
						inventoryTab->reset(game);
					if (has_inventory->inventory == merchantTab->getMerchantInventory())
						merchantTab->reset(game);
				}
			}
		});
		game->signal_player_money_update().connect([this](const PlayerPtr &) {
			inventoryTab->reset(game);
			merchantTab->reset(game);
		});
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
		glClear(GL_COLOR_BUFFER_BIT);
		for (int joystick = GLFW_JOYSTICK_1; joystick <= GLFW_JOYSTICK_LAST; ++joystick)
			if (glfwJoystickPresent(joystick)) {
				int button_count = 0;
				const uint8_t *buttons = glfwGetJoystickButtons(joystick, &button_count);
				// const bool left_pad = buttons[0];
				const bool right_pad = buttons[1];
				const bool a = buttons[2];
				const bool b = buttons[3];
				const bool x = buttons[4];
				const bool y = buttons[5];
				const bool l2Full = buttons[8];
				const bool left_grip  = buttons[15];
				const bool right_grip = buttons[16];
				const bool up    = buttons[17];
				const bool down  = buttons[18];
				const bool left  = buttons[19];
				const bool right = buttons[20];

				int axis_count = 0;
				const float *axes = glfwGetJoystickAxes(joystick, &axis_count);
				if (axes != nullptr && 4 <= axis_count) {
					if (right_pad && canvas) {
						const float rx = axes[2];
						const float ry = axes[3];
						if (!prevRightPad) {
							autofocus = false;
							rightPadStartX = rx;
							rightPadStartY = ry;
							rightPadStartCanvasX = canvas->center.x();
							rightPadStartCanvasY = canvas->center.y();
						} else {
							const float multiplier = 10.f;
							if (rx <= -0.01 || 0.01 <= rx)
								canvas->center.x() = rightPadStartCanvasX + (rx - rightPadStartX) * multiplier;
							if (ry <= -0.01 || 0.01 <= ry)
								canvas->center.y() = rightPadStartCanvasY + (ry - rightPadStartY) * multiplier;
						}
					}

					if (game && game->player) {
						const float lx = axes[0];
						const float ly = axes[1];
						auto &player = *game->player;
						if (ly <= -.5f)
							player.movingUp = true;
						else if (-.1f < ly)
							player.movingUp = false;
						if (.5f <= ly)
							player.movingDown = true;
						else if (ly < .1f)
							player.movingDown = false;
						if (lx <= -.5f)
							player.movingLeft = true;
						else if (-.1f < lx)
							player.movingLeft = false;
						if (.5f <= lx)
							player.movingRight = true;
						else if (lx < .1f)
							player.movingRight = false;
					}
				}

				if (l2Full)
					autofocus = true;

				if (game && game->player) {
					auto &player = *game->player;
					if (!a && prevA)
						player.interactNextTo();

					if (!x && prevX)
						player.interactOn();

					if (up)
						player.movingUp = true;
					else if (prevUp)
						player.movingUp = false;

					if (down)
						player.movingDown = true;
					else if (prevDown)
						player.movingDown = false;

					if (left)
						player.movingLeft = true;
					else if (prevLeft)
						player.movingLeft = false;

					if (right)
						player.movingRight = true;
					else if (prevRight)
						player.movingRight = false;

					if (canvas) {
						if (left_grip)
							canvas->scale /= 1.01f;
						if (right_grip)
							canvas->scale *= 1.01f;
					}
				}

				if (left_grip && right_grip && x)
					get_application()->quit();

				prevA = a;
				prevB = b;
				prevX = x;
				prevY = y;
				prevUp = up;
				prevDown = down;
				prevLeft = left;
				prevRight = right;
				prevRightPad = right_pad;

				// for (int i = 0; i < button_count; ++i) std::cout << i << ':' << int(buttons[i]) << ' ';
				// std::cout << '\n';
			}
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

	void MainWindow::onBlur() {
		if (game && game->player) {
			keyTimes.clear();
			game->player->movingUp    = false;
			game->player->movingRight = false;
			game->player->movingDown  = false;
			game->player->movingLeft  = false;
		}
	}

	void MainWindow::activateContext() {
		glArea.get_context()->make_current();
	}

	bool MainWindow::onKeyPressed(guint keyval, guint keycode, Gdk::ModifierType modifiers) {
		if (!keyTimes.contains(keyval)) {
			handleKey(keyval, keycode, modifiers);
			if (unsigned(modifiers & Gdk::ModifierType::CONTROL_MASK) == 0)
				keyTimes.emplace(keyval, KeyInfo {keycode, modifiers, getTime()});
		} else
			keyTimes.at(keyval).modifiers = modifiers;
		return true;
	}

	void MainWindow::onKeyReleased(guint keyval, guint, Gdk::ModifierType) {
		keyTimes.erase(keyval);

		if (game && game->player) {
			auto &player = *game->player;
			switch (keyval) {
				case GDK_KEY_w: player.movingUp    = false; player.continuousInteraction = false; keyTimes.erase(GDK_KEY_W); break;
				case GDK_KEY_d: player.movingRight = false; player.continuousInteraction = false; keyTimes.erase(GDK_KEY_D); break;
				case GDK_KEY_s: player.movingDown  = false; player.continuousInteraction = false; keyTimes.erase(GDK_KEY_S); break;
				case GDK_KEY_a: player.movingLeft  = false; player.continuousInteraction = false; keyTimes.erase(GDK_KEY_A); break;
				case GDK_KEY_W: player.movingUp    = false; player.continuousInteraction = false; keyTimes.erase(GDK_KEY_w); break;
				case GDK_KEY_D: player.movingRight = false; player.continuousInteraction = false; keyTimes.erase(GDK_KEY_d); break;
				case GDK_KEY_S: player.movingDown  = false; player.continuousInteraction = false; keyTimes.erase(GDK_KEY_s); break;
				case GDK_KEY_A: player.movingLeft  = false; player.continuousInteraction = false; keyTimes.erase(GDK_KEY_a); break;
				case GDK_KEY_Shift_L:
				case GDK_KEY_Shift_R:
					player.continuousInteraction = false;
					break;
				case GDK_KEY_E:
					keyTimes.erase(GDK_KEY_e);
					break;
			}
		}
	}

	void MainWindow::handleKeys() {
		for (auto &[keyval, info]: keyTimes) {
			auto &[keycode, modifiers, time] = info;
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
		if (!canvas)
			return;

		if (game && game->player) {
			auto &player = *game->player;
			const bool control = (modifiers & static_cast<Gdk::ModifierType>(GDK_MODIFIER_MASK)) == Gdk::ModifierType::CONTROL_MASK;

			switch (keyval) {
				case GDK_KEY_S:
				case GDK_KEY_s:
					player.path.clear();
					if (!player.isMoving())
						player.continuousInteraction = keyval == GDK_KEY_S;
					player.movingDown = true;
					return;
				case GDK_KEY_W:
				case GDK_KEY_w:
					player.path.clear();
					if (!player.isMoving())
						player.continuousInteraction = keyval == GDK_KEY_W;
					player.movingUp = true;
					return;
				case GDK_KEY_A:
				case GDK_KEY_a:
					player.path.clear();
					if (!player.isMoving())
						player.continuousInteraction = keyval == GDK_KEY_A;
					player.movingLeft = true;
					return;
				case GDK_KEY_D:
				case GDK_KEY_d:
					player.path.clear();
					if (!player.isMoving())
						player.continuousInteraction = keyval == GDK_KEY_D;
					player.movingRight = true;
					return;
				case GDK_KEY_Shift_L:
				case GDK_KEY_Shift_R:
					if (player.isMoving())
						player.continuousInteraction = true;
					break;
				case GDK_KEY_o: {
					if (game->debugMode) {
						static std::default_random_engine item_rng;
						static const std::array<ItemID, 3> ids {"base:shortsword", "base:red_potion", "base:coins"};
						ItemStack stack(*game, choose(ids, item_rng), 1);
						auto leftover = player.inventory->add(stack);
						if (leftover) {
							auto &realm = *player.getRealm();
							realm.spawn<ItemEntity>(player.position, *leftover);
							realm.getGame().signal_player_inventory_update().emit(game->player);
						}
					}
					return;
				}
					return;
				case GDK_KEY_E:
					game->player->interactOn();
					return;
				case GDK_KEY_e:
				case GDK_KEY_space:
					game->player->interactNextTo();
					return;
				case GDK_KEY_b: {
					const auto rect = game->getVisibleRealmBounds();
					std::cout << '(' << rect.get_x() << ", " << rect.get_y() << " | " << rect.get_width() << " x " << rect.get_height() << ")\n";
					return;
				}
				case GDK_KEY_u:
					glArea.get_context()->make_current();
					game->activeRealm->reupload();
					return;
				case GDK_KEY_f:
					if (control)
						autofocus = !autofocus;
					else
						game->player->focus(*canvas, false);
					return;
				case GDK_KEY_v:
					if (game->debugMode) {
						auto merchant = player.getRealm()->spawn<Merchant>(player.getPosition(), "base:entity/merchant");
						merchant->inventory->add(ItemStack(*game, "base:red_potion", 6));
						merchant->inventory->add(ItemStack(*game, "base:shortsword", 1));
					}
					return;
				case GDK_KEY_t:
					std::cout << "Time: " << int(game->getHour()) << ':' << int(game->getMinute()) << '\n';
					return;
				case GDK_KEY_g:
					if (game->debugMode) {
						try {
							auto house = player.getRealm();
							auto door = house->getTileEntity<Teleporter>();
							const auto house_pos = door->targetPosition + Position(-1, 0);
							auto overworld = game->realms.at(door->targetRealm);
							player.getRealm()->spawn<Miner>(player.getPosition(), overworld->id, house->id, house_pos,
								overworld->closestTileEntity<Building>(house_pos, [](const auto &building) {
									return building->tileID == "base:tile/keep_sw"_id;
								}));
						} catch (const std::exception &err) {
							std::cerr << err.what() << '\n';
						}
					}
					return;
				case GDK_KEY_c:
					if (control) {
						auto *command_dialog = new CommandDialog(*this);
						dialog.reset(command_dialog);
						command_dialog->signal_submit().connect([this](const Glib::ustring &command) {
							auto [success, message] = game->runCommand(command);
							std::cout << "\e[3" << (success? '2' : '1') << "m" <<  message << "\e[39m\n";
						});
						command_dialog->show();
					}
					return;
				case GDK_KEY_0:
				case GDK_KEY_1:
				case GDK_KEY_2:
				case GDK_KEY_3:
				case GDK_KEY_4:
				case GDK_KEY_5:
				case GDK_KEY_6:
				case GDK_KEY_7:
				case GDK_KEY_8:
				case GDK_KEY_9:
					if (game && game->player) {
						game->player->inventory->setActive(keyval == GDK_KEY_0? 9 : keyval - 0x31);
						game->player->inventory->notifyOwner();
					}
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
							error("Error saving game: " + std::string(err.what()));
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

	bool MainWindow::isFocused(const std::shared_ptr<Tab> &tab) const {
		return tab == tabMap.at(notebook.get_nth_page(notebook.get_current_page()));
	}
}
