#pragma once

#include <gtkmm.h>
#include <functional>
#include <list>
#include <memory>
#include <mutex>

#include "ui/DrawingArea.h"

namespace Game3 {
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

		private:
			Glib::RefPtr<Gtk::Builder> builder;
			Glib::RefPtr<Gtk::CssProvider> cssProvider;
			std::list<std::function<void()>> functionQueue;
			std::recursive_mutex functionQueueMutex;
			Glib::Dispatcher functionQueueDispatcher;

			Gtk::Box vbox {Gtk::Orientation::VERTICAL};
			Gtk::Box hbox {Gtk::Orientation::HORIZONTAL};

			Gtk::Button mmx {"--x"}, ppx {"++x"}, mmy {"--y"}, ppy {"++y"};
			DrawingArea drawingArea {*this};
	};
}
