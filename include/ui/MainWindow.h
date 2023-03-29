#pragma once

#include <gtkmm.h>
#include <chrono>
#include <filesystem>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace Game3 {
	class Canvas;
	class CraftingTab;
	class Game;
	class InventoryTab;
	class MerchantTab;
	class TextTab;
	class Tab;

	class MainWindow: public Gtk::ApplicationWindow {
		public:
			std::unique_ptr<Gtk::Dialog> dialog;
			Gtk::HeaderBar *header = nullptr;
			Gtk::Notebook notebook;
			std::shared_ptr<Game> game;
			std::shared_ptr<TextTab> textTab;
			std::shared_ptr<InventoryTab> inventoryTab;
			std::shared_ptr<MerchantTab> merchantTab;
			std::shared_ptr<CraftingTab> craftingTab;
			Gtk::PopoverMenu glMenu;

			MainWindow(BaseObjectType *, const Glib::RefPtr<Gtk::Builder> &);

			static MainWindow * create();

			/** Causes a function to occur on the next GTK tick (or possibly later). Not thread-safe. */
			void delay(std::function<void()>, unsigned count = 1);

			/** Queues a function to be executed in the GTK thread. Thread-safe. Can be used from any thread. */
			void queue(std::function<void()>);

			/** Displays an alert. This will reset the dialog pointer. If you need to use this inside a dialog's code,
			 *  use delay(). */
			void alert(const Glib::ustring &message, Gtk::MessageType = Gtk::MessageType::INFO, bool modal = true,
			           bool use_markup = false);

			/** Displays an error message. (See alert.) */
			void error(const Glib::ustring &message, bool modal = true, bool use_markup = false);

			Glib::RefPtr<Gdk::GLContext> glContext();

			void setStatus(const Glib::ustring &);

			void onBlur();

			void activateContext();

			friend class Canvas;

		private:
			constexpr static std::chrono::milliseconds keyRepeatTime {100};
			constexpr static std::chrono::seconds statusbarExpirationTime {5};
			static std::unordered_map<guint, std::chrono::milliseconds> customKeyRepeatTimes;

			Glib::RefPtr<Gtk::Builder> builder;
			Glib::RefPtr<Gtk::CssProvider> cssProvider;
			std::list<std::function<void()>> functionQueue;
			std::recursive_mutex functionQueueMutex;
			Glib::Dispatcher functionQueueDispatcher;
			Gtk::Paned paned;
			Gtk::Box vbox {Gtk::Orientation::VERTICAL};
			Gtk::Box statusBox {Gtk::Orientation::HORIZONTAL};
			Gtk::GLArea glArea;
			Gtk::Label statusbar;
			Gtk::Label timeLabel;
			Glib::RefPtr<Gio::SimpleAction> debugAction;
			std::unique_ptr<Canvas> canvas;
			std::unordered_map<const Gtk::Widget *, std::shared_ptr<Tab>> tabMap;
			std::shared_ptr<Tab> activeTab;
			double lastDragX = 0.;
			double lastDragY = 0.;
			double glAreaMouseX = 0.;
			double glAreaMouseY = 0.;
			bool autofocus = true;
			bool statusbarWaiting = false;
			std::chrono::system_clock::time_point statusbarSetTime;

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

			void newGame(int seed, int width, int height);
			void loadGame(const std::filesystem::path &);
			void saveGame(const std::filesystem::path &);
			bool render(const Glib::RefPtr<Gdk::GLContext> &);
			bool onKeyPressed(guint, guint, Gdk::ModifierType);
			void onKeyReleased(guint, guint, Gdk::ModifierType);
			void handleKeys();
			void handleKey(guint keyval, guint keycode, Gdk::ModifierType);
			void onNew();
			void connectSave();
			void onGameLoaded();
			bool isFocused(const std::shared_ptr<Tab> &) const;

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
	};
}
