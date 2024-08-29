#pragma once

#include "client/ClientSettings.h"
#include "client/ServerWrapper.h"
#include "threading/MTQueue.h"
#include "threading/Lockable.h"
#include "types/Types.h"
#include "ui/LogOverlay.h"
#include "ui/Modifiers.h"

#include <atomic>
#include <chrono>
#include <concepts>
#include <deque>
#include <filesystem>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include <gtkmm.h>

namespace Game3 {
	class Agent;
	class Canvas;
	class ClientGame;
	class ClientInventory;
	class GTKCraftingTab;
	class GTKInventoryTab;
	class GTKTab;
	class HasFluids;
	class OmniDialog;
	class GTKTextTab;
	struct Position;
	struct WorldGenParams;

	class MainWindow: public Gtk::ApplicationWindow {
		public:
			std::unique_ptr<Gtk::Dialog> dialog;
			Gtk::HeaderBar *header = nullptr;
			Gtk::Notebook notebook;
			std::shared_ptr<ClientGame> game;
			std::shared_ptr<GTKTextTab> textTab;
			std::shared_ptr<GTKInventoryTab> inventoryTab;
			std::shared_ptr<GTKCraftingTab> craftingTab;
			std::shared_ptr<OmniDialog> omniDialog;
			Lockable<ClientSettings> settings;

			MainWindow(BaseObjectType *, const Glib::RefPtr<Gtk::Builder> &);
			~MainWindow() override;

			static MainWindow * create();

			/** Causes a function to occur on the next GTK tick (or possibly later). Not thread-safe. */
			void delay(std::function<void()>, unsigned count = 1);

			/** Queues a function to be executed in the GTK thread. Thread-safe. Can be used from any thread. */
			void queue(std::function<void()>);

			/** Queues a function to be executed in the GTK thread. Thread-safe. Can be used from any thread.
			 *  If the given function returns false, it won't be removed from the queue. */
			void queueBool(std::function<bool()>);

			/** Displays an alert. This will reset the dialog pointer. If you need to use this inside a dialog's code, use delay(). */
			void alert(const Glib::ustring &message, Gtk::MessageType = Gtk::MessageType::INFO, bool queue = true, bool modal = true, bool use_markup = false);

			/** Displays an error message. (See alert.) */
			void error(const Glib::ustring &message, bool queue = true, bool modal = true, bool use_markup = false);

			void closeDialog();
			void queueDialog(std::unique_ptr<Gtk::Dialog> &&);

			void closeGame();

			Glib::RefPtr<Gdk::GLContext> glContext();

			void setStatus(const Glib::ustring &);

			void onBlur();

			bool activateContext();

			/** Focuses the GL area. */
			void yieldFocus();

			/** Sets up a widget to yield focus to the GL area when it receives an esc press. */
			void addYield(Gtk::Widget &);

			void saveSettings();

			void showExternalInventory(const std::shared_ptr<ClientInventory> &);
			GlobalID getExternalGID() const;

			inline auto getActiveTab() const { return activeTab; }

			inline Modifiers getModifiers() const { return glAreaModifiers; }
			Position getHoveredPosition() const;
			inline Gtk::PopoverMenu & getGLMenu() { return glMenu; }

			void openModule(const Identifier &, const std::any &);
			void removeModule();
			void showFluids(const std::shared_ptr<HasFluids> &);

			void moduleMessageBuffer(const Identifier &module_id, const std::shared_ptr<Agent> &source, const std::string &name, Buffer &&data);

			template <typename... Args>
			void moduleMessage(const Identifier &module_id, const std::shared_ptr<Agent> &source, const std::string &name, Args &&...args) {
				moduleMessageBuffer(module_id, source, name, Buffer{std::forward<Args>(args)...});
			}

		private:
			constexpr static std::chrono::milliseconds keyRepeatTime {100};
			constexpr static std::chrono::seconds statusbarExpirationTime {5};
			static std::unordered_map<guint, std::chrono::milliseconds> customKeyRepeatTimes;

			Gtk::Stack stack;
			Gtk::ToggleButton toggleLogButton;
			Glib::RefPtr<Gtk::Builder> builder;
			Glib::RefPtr<Gtk::CssProvider> cssProvider;
			MTQueue<std::function<void()>> functionQueue;
			Lockable<std::list<std::function<bool()>>> boolFunctions;
			Glib::Dispatcher functionQueueDispatcher;
			Glib::Dispatcher boolFunctionDispatcher;
			Gtk::Paned paned;
			Gtk::Box vbox {Gtk::Orientation::VERTICAL};
			Gtk::Box statusBox {Gtk::Orientation::HORIZONTAL};
			Gtk::GLArea glArea;
			Gtk::PopoverMenu glMenu;
			Gtk::Label statusbar;
			Gtk::Label moneyLabel;
			Gtk::Label timeLabel;
			Glib::RefPtr<Gio::SimpleAction> debugAction;
			std::unique_ptr<Canvas> canvas;
			std::unordered_map<const Gtk::Widget *, std::shared_ptr<GTKTab>> tabMap;
			std::shared_ptr<GTKTab> activeTab;
			double lastDragX = 0.;
			double lastDragY = 0.;
			double glAreaMouseX = 0.;
			double glAreaMouseY = 0.;
			Modifiers glAreaModifiers;
			bool autofocus = true;
			bool statusbarWaiting = false;
			std::chrono::system_clock::time_point statusbarSetTime;
			Glib::RefPtr<Gtk::GestureClick> leftClick;
			Glib::RefPtr<Gtk::GestureClick> middleClick;
			Glib::RefPtr<Gtk::GestureClick> rightClick;
			Glib::RefPtr<Gtk::GestureDrag> dragGesture;
			Glib::RefPtr<Gtk::EventControllerMotion> motion;
			std::optional<std::pair<double, double>> dragStart;
			Lockable<std::deque<std::unique_ptr<Gtk::Dialog>>> dialogQueue;
			std::chrono::system_clock::time_point lastRenderTime = std::chrono::system_clock::now();
			std::atomic<double> lastFPS = 0;
			std::deque<double> fpses;
			ServerWrapper serverWrapper;
			LogOverlay logOverlay;

			struct KeyInfo {
				guint code;
				Gdk::ModifierType modifiers;
				std::chrono::system_clock::time_point lastProcessed;
			};

			/** Keys are keyvals, not keycodes. */
			std::map<guint, KeyInfo> keyTimes;

			bool prevA = false;
			bool prevB = false;
			bool prevX = false;
			bool prevY = false;
			bool prevUp    = false;
			bool prevDown  = false;
			bool prevLeft  = false;
			bool prevRight = false;
			bool prevRightPad = false;
			bool prevAutofocus = true;
			float rightPadStartX = 0.f;
			float rightPadStartY = 0.f;
			float rightPadStartCanvasX = 0.f;
			float rightPadStartCanvasY = 0.f;

			bool connect(const Glib::ustring &hostname, uint16_t port);
			bool render(const Glib::RefPtr<Gdk::GLContext> &);
			bool onKeyPressed(guint, guint, Gdk::ModifierType);
			void onKeyReleased(guint, guint, Gdk::ModifierType);
			void handleKeys();
			void handleKey(guint keyval, guint keycode, Gdk::ModifierType);
			void onConnect();
			void autoConnect();
			void playLocally();
			void onGameLoaded();
			bool isFocused(const std::shared_ptr<GTKTab> &) const;
			void connectClose(Gtk::Dialog &);
			void updateMoneyLabel(MoneyCount);
			void continueLocalConnection();
			bool toggleLog();
			const std::shared_ptr<OmniDialog> & getOmniDialog();

			template <typename T>
			T & initTab(std::shared_ptr<T> &tab) {
				tab = std::make_shared<T>(notebook);
				tabMap.emplace(&tab->getWidget(), tab);
				return *tab;
			}

			template <typename T, typename... Args>
			T & initTab(std::shared_ptr<T> &tab, Args && ...args) {
				tab = std::make_shared<T>(std::forward<Args>(args)...);
				tabMap.emplace(&tab->getWidget(), tab);
				return *tab;
			}

		friend class Canvas;
	};
}
