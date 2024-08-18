#include "Log.h"
#include "graphics/Shader.h"
#include "graphics/Tileset.h"

#include "client/RichPresence.h"
#include "entity/ClientPlayer.h"
#include "entity/ItemEntity.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "game/HasInventory.h"
#include "game/Inventory.h"
#include "net/DisconnectedError.h"
#include "net/LocalClient.h"
#include "net/NetError.h"
#include "packet/ContinuousInteractionPacket.h"
#include "packet/LoginPacket.h"
#include "packet/SetHeldItemPacket.h"
#include "realm/Overworld.h"
#include "ui/gl/InventoryDialog.h"
#include "ui/gtk/ConnectDialog.h"
#include "ui/gtk/ConnectionSuccessDialog.h"
#include "ui/gtk/EntryDialog.h"
#include "ui/gtk/JSONDialog.h"
#include "ui/gtk/LoginDialog.h"
#include "ui/gtk/Util.h"
#include "ui/module/InventoryModule.h"
#include "ui/module/FluidLevelsModule.h"
#include "ui/module/ModuleFactory.h"
#include "ui/module/VillageTradeModule.h"
#include "ui/tab/CraftingTab.h"
#include "ui/tab/InventoryTab.h"
#include "ui/tab/TextTab.h"
#include "ui/App.h"
#include "ui/Canvas.h"
#include "ui/LogOverlay.h"
#include "ui/MainWindow.h"
#include "ui/Modifiers.h"
#include "util/FS.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Overworld.h"
#include "worldgen/WorldGen.h"

#include <algorithm>
#include <deque>
#include <fstream>
#include <ranges>

namespace {
	constexpr std::chrono::milliseconds arrowTime {100};
	constexpr std::chrono::milliseconds interactTime {250};
	constexpr std::chrono::milliseconds jumpTime {50};
	constexpr std::chrono::milliseconds slowTime {1'000};
	constexpr std::chrono::milliseconds forever {1'000'000'000};
}

namespace Game3 {
	std::unordered_map<guint, std::chrono::milliseconds> MainWindow::customKeyRepeatTimes {
		{GDK_KEY_Up,           arrowTime},
		{GDK_KEY_Down,         arrowTime},
		{GDK_KEY_Left,         arrowTime},
		{GDK_KEY_Right,        arrowTime},
		{GDK_KEY_q,            interactTime},
		{GDK_KEY_Q,            interactTime},
		{GDK_KEY_e,            interactTime},
		{GDK_KEY_E,            interactTime},
		{GDK_KEY_bracketleft,  interactTime},
		{GDK_KEY_bracketright, interactTime},
		{GDK_KEY_r,            interactTime},
		{GDK_KEY_R,            interactTime},
		{GDK_KEY_o,            interactTime},
		{GDK_KEY_Return,       interactTime},
		{GDK_KEY_space,        jumpTime},
		{GDK_KEY_g,            slowTime},
		{GDK_KEY_0,            slowTime},
		{GDK_KEY_1,            slowTime},
		{GDK_KEY_2,            slowTime},
		{GDK_KEY_3,            slowTime},
		{GDK_KEY_4,            slowTime},
		{GDK_KEY_5,            slowTime},
		{GDK_KEY_6,            slowTime},
		{GDK_KEY_7,            slowTime},
		{GDK_KEY_8,            slowTime},
		{GDK_KEY_9,            slowTime},
		{GDK_KEY_braceleft,    slowTime},
		{GDK_KEY_braceright,   slowTime},
		{GDK_KEY_i,            forever},
	};

	MainWindow::MainWindow(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder_):
	Gtk::ApplicationWindow(cobject), builder(builder_) {
		header = builder->get_widget<Gtk::HeaderBar>("headerbar");
		set_titlebar(*header);

		toggleLogButton.set_icon_name("utilities-terminal-symbolic");
		toggleLogButton.signal_clicked().connect([this] { toggleLog(); });
		header->pack_start(toggleLogButton);

		set_icon_name("game3");
		set_title("Game3");

		cssProvider = Gtk::CssProvider::create();
		cssProvider->load_from_resource("/game3/style.css");
		Gtk::StyleContext::add_provider_for_display(Gdk::Display::get_default(), cssProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

		functionQueueDispatcher.connect([this] {
			for (const auto &fn: functionQueue.steal())
				fn();
		});

		boolFunctionDispatcher.connect([this] {
			auto lock = boolFunctions.uniqueLock();
			std::erase_if(boolFunctions, [](const auto &fn) {
				return fn();
			});
			if (!boolFunctions.empty()) {
				delay([this] {
					boolFunctionDispatcher.emit();
				});
			}
		});

		add_action("connect", Gio::ActionMap::ActivateSlot(sigc::mem_fun(*this, &MainWindow::onConnect)));

		add_action("autoconnect", Gio::ActionMap::ActivateSlot(sigc::mem_fun(*this, &MainWindow::autoConnect)));

		add_action("play_locally", Gio::ActionMap::ActivateSlot(sigc::mem_fun(*this, &MainWindow::playLocally)));

		add_action("toggle_log", [this] { toggleLog(); });

		debugAction = add_action_bool("debug", [this] {
			if (game) {
				game->debugMode = !game->debugMode;
				debugAction->set_state(Glib::Variant<bool>::create(game->debugMode));
			}
		}, false);

		add_action("settings", Gio::ActionMap::ActivateSlot([this] {
			auto dialog = settings.makeDialog(*this, [this](const ClientSettings &new_settings) {
				settings = new_settings;
				if (game)
					settings.apply(*game);
				settings.apply();
				saveSettings();
			});
			queueDialog(std::move(dialog));
		}));

		glMenu.set_parent(glArea);
		glArea.set_expand(true);
		glArea.set_use_es(false);
		glArea.signal_realize().connect([this] {
			glArea.make_current();
			INFOX(3, "Using ES: {}", glArea.get_context()->get_use_es());
			glArea.throw_if_error();
			canvas = std::make_unique<Canvas>(*this);
		});
		glArea.signal_unrealize().connect([this] {
			glArea.make_current();
			glArea.throw_if_error();
		});
		glArea.signal_render().connect(sigc::mem_fun(*this, &MainWindow::render), false);
		glArea.set_auto_render(true);
		glArea.add_tick_callback([this](const Glib::RefPtr<Gdk::FrameClock> &) {
			glArea.queue_render();

			if (game) {
				PlayerPtr player = game->getPlayer();
				if (autofocus && player)
					player->focus(*canvas, true);
				timeLabel.set_text(std::format("{}:{:02}", int(game->getHour()), int(game->getMinute())));
			}

			fpses.push_back(lastFPS.load());
			if (100 < fpses.size())
				fpses.pop_front();

			double sum = 0;
			for (double fps: fpses)
				sum += fps;

			statusbar.set_text(std::to_string(int(sum / fpses.size())) + " FPS");

			if (statusbarWaiting && statusbarExpirationTime <= getTime() - statusbarSetTime) {
				statusbarWaiting = false;
				statusbar.set_text({});
			}

			return true;
		});
		glArea.set_focusable(true);

		auto key_controller = Gtk::EventControllerKey::create();
		key_controller->signal_key_pressed().connect(sigc::mem_fun(*this, &MainWindow::onKeyPressed), true);
		key_controller->signal_key_released().connect(sigc::mem_fun(*this, &MainWindow::onKeyReleased));
		add_controller(key_controller);

		auto drag = Gtk::GestureDrag::create();
		drag->set_button(2);
		drag->signal_drag_begin().connect([this](double, double) {
			lastDragX = lastDragY = 0;
			autofocus = false;
		});
		drag->signal_drag_update().connect([this](double x, double y) {
			double delta_x = x - lastDragX;
			double delta_y = y - lastDragY;
			lastDragX = x;
			lastDragY = y;
			if (canvas) {
				canvas->center.first  += delta_x / (canvas->magic * canvas->scale / canvas->getFactor());
				canvas->center.second += delta_y / (canvas->magic * canvas->scale / canvas->getFactor());
			}
		});
		glArea.add_controller(drag);

		motion = Gtk::EventControllerMotion::create();
		motion->signal_motion().connect([this](double x, double y) {
			glAreaMouseX = x;
			glAreaMouseY = y;
			glAreaModifiers = Modifiers(motion->get_current_event_state());
		});
		glArea.add_controller(motion);

		leftClick   = Gtk::GestureClick::create();
		middleClick = Gtk::GestureClick::create();
		rightClick  = Gtk::GestureClick::create();
		leftClick->set_button(1);
		middleClick->set_button(2);
		rightClick->set_button(3);
		leftClick->signal_released().connect([this](int n, double x, double y) {
			glArea.grab_focus();
			if (game)
				game->click(1, n, x, y, Modifiers(leftClick->get_current_event_state()));
		});
		middleClick->signal_released().connect([this](int, double, double) {
			glArea.grab_focus();
		});
		rightClick->signal_released().connect([this](int n, double x, double y) {
			glArea.grab_focus();
			if (game)
				game->click(3, n, x, y, Modifiers(rightClick->get_current_event_state()));
		});
		glArea.add_controller(leftClick);
		glArea.add_controller(middleClick);
		glArea.add_controller(rightClick);

		dragGesture = Gtk::GestureDrag::create();
		dragGesture->signal_drag_begin().connect([this](double x, double y) {
			glArea.grab_focus();
			dragStart.emplace(x, y);
			if (game)
				game->dragStart(game->translateCanvasCoordinates(x, y), Modifiers(dragGesture->get_current_event_state()));
		});
		dragGesture->signal_drag_update().connect([this](double x, double y) {
			glArea.grab_focus();
			if (game && autofocus) {
				assert(dragStart);
				game->dragUpdate(game->translateCanvasCoordinates(dragStart->first + x, dragStart->second + y), Modifiers(dragGesture->get_current_event_state()));
			}
		});
		dragGesture->signal_drag_end().connect([this](double x, double y) {
			glArea.grab_focus();
			if (game)
				game->dragEnd(game->translateCanvasCoordinates(dragStart->first + x, dragStart->second + y), Modifiers(dragGesture->get_current_event_state()));
			dragStart.reset();
		});
		glArea.add_controller(dragGesture);

		glArea.add_controller(createClick([this] {
			if (!game)
				return;

			if (PlayerPtr player = game->getPlayer()) {
				player->getInventory(0)->nextSlot();
				inventoryTab->update(game);
			}
		}, 9, true));

		glArea.add_controller(createClick([this] {
			if (!game)
				return;

			if (PlayerPtr player = game->getPlayer()) {
				player->getInventory(0)->prevSlot();
				inventoryTab->update(game);
			}
		}, 8, true));

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
			canvas->center.first -= difference_x * side_ratio_x / 8.f * canvas->getFactor();

			const auto difference_y = h / old_scale - h / canvas->scale;
			const auto side_ratio_y = (glAreaMouseY - h / 2.f) / h;
			canvas->center.second -= difference_y * side_ratio_y / 8.f * canvas->getFactor();

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

		moneyLabel.set_xalign(0.0);
		moneyLabel.set_size_request(128, -1);
		statusBox.append(moneyLabel);

		timeLabel.set_xalign(1.0);
		timeLabel.set_size_request(42, -1);
		timeLabel.set_margin_end(10);
		statusBox.append(timeLabel);

		vbox.append(statusBox);

		paned.set_orientation(Gtk::Orientation::HORIZONTAL);
		paned.set_start_child(vbox);
		paned.set_end_child(notebook);

		paned.set_resize_start_child(true);
		paned.set_shrink_start_child(false);
		paned.set_resize_end_child(false);
		paned.set_shrink_end_child(false);
		paned.property_position().signal_changed().connect([this] {
			tabMap.at(notebook.get_nth_page(notebook.get_current_page()))->onResize(game);
		});

		glArea.set_expand(true);

		notebook.set_hexpand(false);
		notebook.set_vexpand(true);
		notebook.property_page().signal_changed().connect([this] {
			if (activeTab)
				activeTab->onBlur();
			activeTab = tabMap.at(notebook.get_nth_page(notebook.get_current_page()));
			activeTab->onFocus();
		});

		initTab(inventoryTab, *this).add();
		initTab(craftingTab, *this).add();
		initTab(textTab, notebook, "", "");
		activeTab = inventoryTab;

		stack.add(paned);
		stack.add(logOverlay);

		set_child(stack);
		delay([this] {
			paned.set_position(paned.get_width() - 365);
		}, 2);

		if (std::filesystem::exists("settings.json"))
			settings = nlohmann::json::parse(readFile("settings.json"));

		settings.apply();
	}

	MainWindow::~MainWindow() {
		glMenu.unparent();
		if (game)
			game->stopThread();
	}

	bool MainWindow::connect(const Glib::ustring &hostname, uint16_t port) {
		closeGame();
		game = std::dynamic_pointer_cast<ClientGame>(Game::create(Side::Client, canvas.get()));
		auto client = std::make_shared<LocalClient>();
		game->setClient(client);
		try {
			client->connect(hostname.raw(), port);
		} catch (const std::exception &err) {
			closeGame();
			error(err.what());
			return false;
		}
		client->weakGame = game;
		game->initEntities();

		// It's assumed that the caller already owns a unique lock on the settings.
		settings.hostname = hostname;
		settings.port = port;

		if (std::filesystem::exists("tokens.json"))
			client->readTokens("tokens.json");
		else
			client->saveTokens("tokens.json");

		activateContext();
		onGameLoaded();

		if (settings.alertOnConnection) {
			auto success_dialog = std::make_unique<ConnectionSuccessDialog>(*this);
			success_dialog->signal_submit().connect([this](bool checked) {
				auto lock = settings.uniqueLock();
				settings.alertOnConnection = !checked;
			});
			queueDialog(std::move(success_dialog));
		}

		return true;
	}

	void MainWindow::onGameLoaded() {
		richPresence.setActivityStartTime(false);
		richPresence.setActivityDetails("Playing", true);

		debugAction->set_state(Glib::Variant<bool>::create(game->debugMode));
		game->initInteractionSets();
		canvas->game = game;
		settings.apply(*game);

		for (auto &[widget, tab]: tabMap)
			tab->reset(game);

		game->signalPlayerInventoryUpdate().connect([this](const PlayerPtr &player) {
			if (player != game->getPlayer())
				return;
			inventoryTab->update(game);
			craftingTab->update(game);
		});

		game->signalOtherInventoryUpdate().connect([this](const std::shared_ptr<Agent> &owner, InventoryID inventory_id) {
			if (auto has_inventory = std::dynamic_pointer_cast<HasInventory>(owner); has_inventory && has_inventory->getInventory(inventory_id)) {
				auto client_inventory = std::dynamic_pointer_cast<ClientInventory>(has_inventory->getInventory(inventory_id));
				queue([this, owner, client_inventory] {
					if (owner->getGID() == getExternalGID()) {
						std::unique_lock<DefaultMutex> lock;
						if (auto *module_ = inventoryTab->getModule(lock))
							module_->setInventory(client_inventory);
					}
				});
			}
		});

		game->signalPlayerMoneyUpdate().connect([this](const PlayerPtr &player) {
			updateMoneyLabel(player->getMoney());
		});

		game->signalFluidUpdate().connect([this](const std::shared_ptr<HasFluids> &has_fluids) {
			queue([this, has_fluids] {
				std::unique_lock<DefaultMutex> lock;
				if (Module *module_ = inventoryTab->getModule(lock)) {
					std::any data(has_fluids);
					module_->handleMessage({}, "UpdateFluids", data);
				}
			});
		});

		game->signalEnergyUpdate().connect([this](const std::shared_ptr<HasEnergy> &has_energy) {
			queue([this, has_energy] {
				std::unique_lock<DefaultMutex> lock;
				if (Module *module_ = inventoryTab->getModule(lock)) {
					std::any data(has_energy);
					module_->handleMessage({}, "UpdateEnergy", data);
				}
			});
		});

		game->signalVillageUpdate().connect([this](const VillagePtr &village) {
			queue([this, village] {
				std::unique_lock<DefaultMutex> lock;
				if (Module *module_ = inventoryTab->getModule(lock)) {
					std::any data(village);
					module_->handleMessage({}, "VillageUpdate", data);
				}
			});
		});

		game->errorCallback = [this] {
			if (game->suppressDisconnectionMessage)
				return;

			queue([this] {
				get_display()->beep();
				error("Game disconnected.");
				closeGame();
			});
		};

		game->startThread();
	}

	void MainWindow::connectClose(Gtk::Dialog &to_connect) {
		to_connect.signal_hide().connect([this] {
			queue([this] {
				closeDialog();
			});
		}, true);
	}

	void MainWindow::updateMoneyLabel(MoneyCount money) {
		moneyLabel.set_text(std::format("฿ {}", money));
	}

	bool MainWindow::render(const Glib::RefPtr<Gdk::GLContext> &context) {
		auto now = std::chrono::system_clock::now();
		lastFPS = 1e9 / std::chrono::duration_cast<std::chrono::nanoseconds>(now - lastRenderTime).count();
		lastRenderTime = now;

		context->make_current();

		richPresence.tick();

		glArea.throw_if_error();

		CHECKGL

		if (autofocus && game)
			if (PlayerPtr player = game->getPlayer())
				player->focus(*canvas, true);
		canvas->drawGL();
		return true;
	}

	MainWindow * MainWindow::create() {
		auto builder = Gtk::Builder::create_from_resource("/game3/window.ui");
		auto *window = Gtk::Builder::get_widget_derived<MainWindow>(builder, "main_window");
		if (!window)
			throw std::runtime_error("No \"main_window\" object in window.ui");
		return window;
	}

	void MainWindow::delay(std::function<void()> fn, unsigned count) {
		if (count <= 1) {
			add_tick_callback([fn](const auto &) {
				fn();
				return false;
			});
		} else {
			delay([this, fn, count] {
				delay(fn, count - 1);
			});
		}
	}

	void MainWindow::queue(std::function<void()> fn) {
		functionQueue.push(std::move(fn));
		functionQueueDispatcher.emit();
	}

	void MainWindow::queueBool(std::function<bool()> fn) {
		{
			auto lock = boolFunctions.uniqueLock();
			boolFunctions.push_back(std::move(fn));
		}
		boolFunctionDispatcher.emit();
	}

	void MainWindow::alert(const Glib::ustring &message, Gtk::MessageType type, bool do_queue, bool modal, bool use_markup) {
		// This is a horrible hack to avoid C++23.
		auto new_dialog = std::make_shared<std::unique_ptr<Gtk::MessageDialog>>(std::make_unique<Gtk::MessageDialog>(*this, message, use_markup, type, Gtk::ButtonsType::OK, modal));

		(*new_dialog)->signal_response().connect([this](int) {
			closeDialog();
		});

		if (do_queue) {
			queue([this, new_dialog] {
				queueDialog(std::move(*new_dialog));
			});
		} else {
			dialog = std::move(*new_dialog);
			connectClose(*dialog);
			dialog->show();
		}
	}

	void MainWindow::error(const Glib::ustring &message, bool do_queue, bool modal, bool use_markup) {
		alert(message, Gtk::MessageType::ERROR, do_queue, modal, use_markup);
	}

	void MainWindow::closeDialog() {
		dialog.reset();
		auto lock = dialogQueue.uniqueLock();
		if (!dialogQueue.empty()) {
			dialog = std::move(dialogQueue.front());
			dialogQueue.pop_front();
			dialog->show();
		}
	}

	void MainWindow::queueDialog(std::unique_ptr<Gtk::Dialog> &&new_dialog) {
		if (!dialog) {
			dialog = std::move(new_dialog);
			connectClose(*dialog);
			dialog->show();
		} else {
			auto lock = dialogQueue.uniqueLock();
			connectClose(*new_dialog);
			dialogQueue.push_back(std::move(new_dialog));
		}
	}

	void MainWindow::closeGame() {
		if (game) {
			richPresence.setActivityDetails("Idling");

			if (dialog)
				dialog->close();

			inventoryTab->reset(nullptr);
			removeModule();
			game->stopThread();
			canvas->game = nullptr;
			for (const auto &[widget, tab]: tabMap)
				tab->reset(nullptr);
			game = nullptr;
		}
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
		keyTimes.clear();

		if (!game)
			return;

		if (PlayerPtr player = game->getPlayer())
			player->stopMoving();
	}

	bool MainWindow::activateContext() {
		if (stack.get_visible_child() != &paned)
			return false;

		glArea.get_context()->make_current();
		return true;
	}

	void MainWindow::yieldFocus() {
		glArea.grab_focus();
	}

	void MainWindow::addYield(Gtk::Widget &widget) {
		auto controller = Gtk::EventControllerKey::create();
		controller->signal_key_pressed().connect([this](guint keyval, guint, Gdk::ModifierType) {
			if (keyval == GDK_KEY_Escape) {
				game->getWindow().yieldFocus();
				return true;
			}

			return false;
		}, false);
		widget.add_controller(controller);
	}

	void MainWindow::saveSettings() {
		std::ofstream ofs("settings.json");
		nlohmann::json json;
		{
			auto lock = settings.sharedLock();
			json = settings;
		}
		ofs << json.dump();
	}

	void MainWindow::showExternalInventory(const std::shared_ptr<ClientInventory> &inventory) {
		assert(inventory);
		inventoryTab->setModule(std::make_shared<InventoryModule>(game, inventory));
	}

	GlobalID MainWindow::getExternalGID() const {
		std::unique_lock<DefaultMutex> lock;

		if (Module *module_ = inventoryTab->getModule(lock)) {
			std::any empty;
			if (std::optional<Buffer> response = module_->handleMessage({}, "GetAgentGID", empty))
				return response->take<GlobalID>();
		}

		return -1;
	}

	Position MainWindow::getHoveredPosition() const {
		assert(game);
		return game->translateCanvasCoordinates(glAreaMouseX, glAreaMouseY);
	}

	void MainWindow::openModule(const Identifier &module_id, const std::any &argument) {
		std::unique_lock<DefaultMutex> module_lock;
		Module *current_module = inventoryTab->getModule(module_lock);
		if (current_module != nullptr && current_module->getID() == module_id) {
			current_module->update();
		} else {
			if (current_module != nullptr)
				module_lock.unlock();
			auto &registry = game->registry<ModuleFactoryRegistry>();
			inventoryTab->setModule((*registry[module_id])(game, argument));
		}
	}

	void MainWindow::removeModule() {
		inventoryTab->removeModule();
	}

	void MainWindow::moduleMessageBuffer(const Identifier &module_id, const std::shared_ptr<Agent> &source, const std::string &name, Buffer &&buffer) {
		std::unique_lock<DefaultMutex> module_lock;
		Module *current_module = inventoryTab->getModule(module_lock);
		if (current_module != nullptr && (module_id.empty() || current_module->getID() == module_id)) {
			std::any data{std::move(buffer)};
			current_module->handleMessage(source, name, data);
		}
	}

	void MainWindow::showFluids(const std::shared_ptr<HasFluids> &has_fluids) {
		inventoryTab->setModule(std::make_unique<FluidLevelsModule>(game, has_fluids));
	}

	bool MainWindow::onKeyPressed(guint keyval, guint keycode, Gdk::ModifierType modifiers) {
		if (dynamic_cast<Gtk::Text *>(get_focus()))
			return false;

		if (!keyTimes.contains(keyval)) {
			handleKey(keyval, keycode, modifiers);
			if (unsigned(modifiers & Gdk::ModifierType::CONTROL_MASK) == 0)
				keyTimes.emplace(keyval, KeyInfo{keycode, modifiers, getTime()});
		} else
			keyTimes.at(keyval).modifiers = modifiers;

		return true;
	}

	void MainWindow::onKeyReleased(guint keyval, guint, Gdk::ModifierType) {
		keyTimes.erase(keyval);

		if (game && game->getPlayer()) {
			ClientPlayerPtr player = game->getPlayer();
			switch (keyval) {
				case GDK_KEY_w: player->stopMoving(Direction::Up);    player->stopContinuousInteraction(); keyTimes.erase(GDK_KEY_W); break;
				case GDK_KEY_d: player->stopMoving(Direction::Right); player->stopContinuousInteraction(); keyTimes.erase(GDK_KEY_D); break;
				case GDK_KEY_s: player->stopMoving(Direction::Down);  player->stopContinuousInteraction(); keyTimes.erase(GDK_KEY_S); break;
				case GDK_KEY_a: player->stopMoving(Direction::Left);  player->stopContinuousInteraction(); keyTimes.erase(GDK_KEY_A); break;
				case GDK_KEY_W: player->stopMoving(Direction::Up);    player->stopContinuousInteraction(); keyTimes.erase(GDK_KEY_w); break;
				case GDK_KEY_D: player->stopMoving(Direction::Right); player->stopContinuousInteraction(); keyTimes.erase(GDK_KEY_d); break;
				case GDK_KEY_S: player->stopMoving(Direction::Down);  player->stopContinuousInteraction(); keyTimes.erase(GDK_KEY_s); break;
				case GDK_KEY_A: player->stopMoving(Direction::Left);  player->stopContinuousInteraction(); keyTimes.erase(GDK_KEY_a); break;
				case GDK_KEY_Shift_L:
				case GDK_KEY_Shift_R:
					player->continuousInteraction = false;
					break;
				case GDK_KEY_E:
					keyTimes.erase(GDK_KEY_e);
					break;
				case GDK_KEY_e:
					keyTimes.erase(GDK_KEY_E);
					break;
				case GDK_KEY_R:
					keyTimes.erase(GDK_KEY_r);
					break;
				case GDK_KEY_r:
					keyTimes.erase(GDK_KEY_R);
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

		const bool control = (modifiers & static_cast<Gdk::ModifierType>(GDK_MODIFIER_MASK)) == Gdk::ModifierType::CONTROL_MASK;

		if (game) {
			switch (keyval) {
				case GDK_KEY_c:
					if (control) {
						auto command_dialog = std::make_unique<EntryDialog>(*this, "Command:");
						command_dialog->signal_submit().connect([this](const Glib::ustring &command) {
							try {
								game->runCommand(command);
							} catch (const std::exception &err) {
								error(err.what());
							}
						});
						queueDialog(std::move(command_dialog));
						return;
					}
					break;
				default:
					break;
			}
		}

		if (game && game->getPlayer()) {
			ClientPlayerPtr player = game->getPlayer();

			auto handle_movement = [&](guint kv, Direction direction) {
				if (!player->isMoving())
					player->setContinuousInteraction(keyval == kv, Modifiers(modifiers));
				if (!player->isMoving(direction))
					player->startMoving(direction);
			};

			switch (keyval) {
				case GDK_KEY_S:
				case GDK_KEY_s:
					handle_movement(GDK_KEY_S, Direction::Down);
					return;
				case GDK_KEY_W:
				case GDK_KEY_w:
					handle_movement(GDK_KEY_W, Direction::Up);
					return;
				case GDK_KEY_A:
				case GDK_KEY_a:
					handle_movement(GDK_KEY_A, Direction::Left);
					return;
				case GDK_KEY_D:
				case GDK_KEY_d:
					handle_movement(GDK_KEY_D, Direction::Right);
					return;
				case GDK_KEY_Shift_L:
				case GDK_KEY_Shift_R:
					if (player->isMoving())
						player->send(ContinuousInteractionPacket(player->continuousInteractionModifiers));
					return;
				case GDK_KEY_E:
					game->interactNextTo(Modifiers(modifiers) | Modifiers(true, false, false, false), Hand::Right);
					return;
				case GDK_KEY_e:
				case GDK_KEY_Return:
					game->interactNextTo(Modifiers(modifiers), Hand::Right);
					return;
				case GDK_KEY_Q:
					game->interactNextTo(Modifiers(modifiers) | Modifiers(true, false, false, false), Hand::Left);
					return;
				case GDK_KEY_q:
					game->interactNextTo(Modifiers(modifiers), Hand::Left);
					return;
				case GDK_KEY_braceleft:
				case GDK_KEY_braceright:
					player->send(SetHeldItemPacket(keyval == GDK_KEY_braceleft, player->getActiveSlot()));
					return;
				case GDK_KEY_R:
					game->interactOn(Modifiers(modifiers) | Modifiers(true, false, false, false));
					return;
				case GDK_KEY_r:
					game->interactOn(Modifiers(modifiers));
					return;
				case GDK_KEY_space:
					player->jump();
					return;
				case GDK_KEY_b: {
					const auto rect = game->getVisibleRealmBounds();
					INFO("({}y, {}x) x ({}h, {}w)", rect.get_y(), rect.get_x(), rect.get_height(), rect.get_width());
					return;
				}
				case GDK_KEY_m: {
					if (player->isMoving()) {
						std::vector<const char *> moves;
						if (player->movingUp) moves.push_back("up");
						if (player->movingDown) moves.push_back("down");
						if (player->movingLeft) moves.push_back("left");
						if (player->movingRight) moves.push_back("right");
						if (moves.empty())
							moves.push_back("nowhere");
						std::stringstream ss;
						bool first = true;
						for (const char *move: moves) {
							if (first)
								first = false;
							else
								ss << ", ";
							ss << move;
						}
						INFO("Player is moving {}. Offset: {}", ss.str(), player->offset);
					} else {
						INFO("Player isn't moving. Offset: {}", player->offset);
					}
					return;
				}
				case GDK_KEY_i:
					if (canvas->uiContext.removeDialogs<InventoryDialog>() == 0) {
						canvas->uiContext.addDialog<InventoryDialog>();
					}
					return;
				case GDK_KEY_u:
					if (control) {
						game->runCommand("usage");
					} else if (activateContext()) {
						game->getActiveRealm()->reupload();
					}
					return;
				case GDK_KEY_f:
					if (control)
						autofocus = !autofocus;
					else
						game->getPlayer()->focus(*canvas, false);
					return;
				case GDK_KEY_t:
					if (Modifiers(modifiers).ctrl) {
						RealmPtr realm = game->getPlayer()->getRealm();
						const auto &chunk = realm->tileProvider.getTileChunk(Layer::Terrain, game->getPlayer()->getChunk());
						auto lock = chunk.sharedLock();
						for (size_t row = 0; row < CHUNK_SIZE; ++row) {
							for (size_t column = 0; column < CHUNK_SIZE; ++column)
								std::cout << std::setw(4) << std::right << chunk.at(row * CHUNK_SIZE + column);
							std::cout << '\n';
						}
					} else {
						std::cout << "Time: " << int(game->getHour()) << ':' << int(game->getMinute()) << '\n';
					}
					return;
				case GDK_KEY_T: {
					auto realm = game->getPlayer()->getRealm();
					const Position where = game->getPlayer()->getPosition() + game->getPlayer()->direction.load();
					if (auto tile_entity = realm->tileEntityAt(where))
						INFO("TileEntity: {} {}", tile_entity->tileEntityID, tile_entity->getGID());
					else
						INFO("No TileEntity found at {}", where);
					return;
				}
				case GDK_KEY_p: {
					ClientPlayerPtr player = game->getPlayer();
					std::cout << std::format("Player GID: {}\n", player->getGID());
					std::cout << std::format("Realm ID: {} or perhaps {}\n", player->getRealm()->getID(), game->getActiveRealm()->getID());
					std::cout << std::format("Position: {}\n", player->getPosition());
					std::cout << std::format("Chunk position: {}\n", player->getPosition().getChunk());
					std::cout << std::format("Update counter: {}\n", player->getRealm()->tileProvider.getUpdateCounter(player->getPosition().getChunk()));
					std::cout << std::format("Canvas scale: {}\n", canvas->scale);
					std::cout << std::format("Outdoors: {}\n", player->getRealm()->outdoors);
					return;
				}
				case GDK_KEY_l:
					if (RealmPtr active_realm = game->getActiveRealm())
						active_realm->queueStaticLightingTexture();
					break;
				case GDK_KEY_minus:
					canvas->scale /= 1.08f;
					return;
				case GDK_KEY_equal:
				case GDK_KEY_plus:
					canvas->scale *= 1.08f;
					return;
				case GDK_KEY_0: case GDK_KEY_1: case GDK_KEY_2: case GDK_KEY_3: case GDK_KEY_4:
				case GDK_KEY_5: case GDK_KEY_6: case GDK_KEY_7: case GDK_KEY_8: case GDK_KEY_9:
					if (game && game->getPlayer())
						game->getPlayer()->getInventory(0)->setActive(keyval == GDK_KEY_0? 9 : keyval - 0x31);
					return;
			}

			switch (keyval) {
				case GDK_KEY_Down:
					game->getPlayer()->face(Direction::Down);
					break;
				case GDK_KEY_Up:
					game->getPlayer()->face(Direction::Up);
					break;
				case GDK_KEY_Left:
					game->getPlayer()->face(Direction::Left);
					break;
				case GDK_KEY_Right:
					game->getPlayer()->face(Direction::Right);
					break;
				default:
					break;
			}
		}

		if (!autofocus) {
			const float delta = canvas->scale / 20.f;
			switch (keyval) {
				case GDK_KEY_Down:
					canvas->center.second -= delta;
					break;
				case GDK_KEY_Up:
					canvas->center.second += delta;
					break;
				case GDK_KEY_Left:
					canvas->center.first += delta;
					break;
				case GDK_KEY_Right:
					canvas->center.first -= delta;
					break;
				default:
					break;
			}
		}
	}

	void MainWindow::onConnect() {
		auto connect_dialog = std::make_unique<ConnectDialog>(*this);
		connect_dialog->set_transient_for(*this);
		connect_dialog->signal_submit().connect([this](const auto &hostname, uint16_t port) {
			MainWindow::connect(hostname, port);
		});
		queueDialog(std::move(connect_dialog));
	}

	void MainWindow::autoConnect() {
		auto lock = settings.uniqueLock();

		if (settings.username.empty()) {
			error("No username set. Try logging in manually first.");
			return;
		}

		if (!connect(settings.hostname, settings.port))
			return;

		LocalClientPtr client = game->getClient();
		const std::string &hostname = client->getHostname();
		if (std::optional<Token> token = client->getToken(hostname, settings.username)) {
			queueBool([this, client, token, hostname, username = settings.username] {
				if (game && client && client->isReady()) {
					client->send(LoginPacket(username, *token));
					return true;
				}

				return false;
			});
		} else {
			error("Couldn't find token for user " + settings.username + " on host " + hostname);
		}
	}

	void MainWindow::playLocally() {
		const bool was_running = serverWrapper.isRunning();

		if (game) {
			game->suppressDisconnectionMessage = true;
		}

		serverWrapper.onLog = [this](std::string_view text) {
			logOverlay.print(text);
		};

		size_t seed = 1621;
		if (std::filesystem::exists(".seed")) {
			try {
				seed = parseNumber<size_t>(trim(readFile(".seed")));
				INFO("Using custom seed \e[1m{}\e[22m", seed);
			} catch (const std::exception &err) {
				ERROR("Failed to load seed from .seed: {}", err.what());
			}
		}

		serverWrapper.runInThread(seed);

		if (!serverWrapper.waitUntilRunning(std::chrono::milliseconds(10'000))) {
			error("Server failed to start within 10 seconds.");
			return;
		}

		serverWrapper.save();

		if (was_running) {
			// Ugly hack!
			delay([this] { continueLocalConnection(); }, DEFAULT_CLIENT_TICK_FREQUENCY * 2);
		} else {
			continueLocalConnection();
		}
	}

	void MainWindow::continueLocalConnection() {
		if (!connect("::1", serverWrapper.getPort())) {
			error("Failed to connect to local server.");
			return;
		}

		assert(game);
		LocalClientPtr client = game->getClient();

		auto login_dialog = std::make_unique<LoginDialog>(*this, settings.username);
		login_dialog->signal_submit().connect([this, weak_client = std::weak_ptr(client)](const Glib::ustring &username, const Glib::ustring &display_name) {
			if (LocalClientPtr client = weak_client.lock())
				client->send(LoginPacket(username.raw(), serverWrapper.getOmnitoken(), display_name));
		});
		queueDialog(std::move(login_dialog));
	}

	bool MainWindow::toggleLog() {
		if (stack.get_visible_child() == &paned) {
			stack.set_visible_child(logOverlay);
			toggleLogButton.set_active(true);
			return true;
		}

		stack.set_visible_child(paned);
		toggleLogButton.set_active(false);
		return false;
	}

	bool MainWindow::isFocused(const std::shared_ptr<Tab> &tab) const {
		return tab == tabMap.at(notebook.get_nth_page(notebook.get_current_page()));
	}
}
