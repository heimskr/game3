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
	class Game;
	class InventoryTab;
	class Tab;

	class MainWindow: public Gtk::ApplicationWindow {
		public:
			std::unique_ptr<Gtk::Dialog> dialog;
			Gtk::HeaderBar *header = nullptr;
			std::shared_ptr<Game> game;

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

			friend class Canvas;

		private:
			constexpr static std::chrono::milliseconds keyRepeatTime {100};
			static std::unordered_map<guint, std::chrono::milliseconds> customKeyRepeatTimes;

			Glib::RefPtr<Gtk::Builder> builder;
			Glib::RefPtr<Gtk::CssProvider> cssProvider;
			std::list<std::function<void()>> functionQueue;
			std::recursive_mutex functionQueueMutex;
			Glib::Dispatcher functionQueueDispatcher;
			Gtk::Paned paned;
			Gtk::Notebook notebook;
			Gtk::GLArea glArea;
			std::unique_ptr<Canvas> canvas;
			std::unordered_map<Gtk::Widget *, std::shared_ptr<Tab>> tabMap;
			std::shared_ptr<Tab> activeTab;
			std::shared_ptr<InventoryTab> inventoryTab;
			double lastDragX = 0.;
			double lastDragY = 0.;
			double glAreaMouseX = 0.;
			double glAreaMouseY = 0.;
			bool autoFocus = true;

			struct KeyInfo {
				guint value;
				Gdk::ModifierType modifiers;
				std::chrono::system_clock::time_point lastProcessed;
			};

			/** Keys are keycodes, not keyvals. */
			std::map<guint, KeyInfo> keyTimes;

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
			void addTab(std::shared_ptr<Tab>);
			void onGameLoaded();
	};
}
