#pragma once

#include <gtkmm.h>
#include <functional>
#include <list>
#include <memory>
#include <mutex>

#include "ui/Canvas.h"

namespace Game3 {
	class Game;

	class MainWindow: public Gtk::ApplicationWindow {
		public:
			std::unique_ptr<Gtk::Dialog> dialog;
			Gtk::HeaderBar *header = nullptr;

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

			friend class Canvas;

		private:
			Glib::RefPtr<Gtk::Builder> builder;
			Glib::RefPtr<Gtk::CssProvider> cssProvider;
			std::list<std::function<void()>> functionQueue;
			std::recursive_mutex functionQueueMutex;
			Glib::Dispatcher functionQueueDispatcher;
			Gtk::GLArea glArea;
			std::unique_ptr<Canvas> canvas;
			std::shared_ptr<Game> game;
			int oldWidth = 0;
			int oldHeight = 0;
			bool queueReupload = false;

			void newGame(int seed, int width, int height);
			bool render(const Glib::RefPtr<Gdk::GLContext> &);
			bool onKey(guint, guint, Gdk::ModifierType);
	};
}
