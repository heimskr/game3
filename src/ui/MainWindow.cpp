#include "ui/MainWindow.h"

namespace Game3 {
	MainWindow::MainWindow(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder_):
	Gtk::ApplicationWindow(cobject), builder(builder_) {
		header = builder->get_widget<Gtk::HeaderBar>("headerbar");
		set_titlebar(*header);

		cssProvider = Gtk::CssProvider::create();
		cssProvider->load_from_resource("/game3/style.css");
		Gtk::StyleContext::add_provider_for_display(Gdk::Display::get_default(), cssProvider,
			GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

		set_child(vbox);
		vbox.append(hbox);
		vbox.append(sfmlArea);
		hbox.append(mmx);
		hbox.append(ppx);
		hbox.append(mmy);
		hbox.append(ppy);
		hbox.append(draw);
		sfmlArea.set_hexpand(true);
		sfmlArea.set_vexpand(true);

		mmx.signal_clicked().connect([this] { --drawingArea.x_; drawingArea.queue_draw(); });
		ppx.signal_clicked().connect([this] { ++drawingArea.x_; drawingArea.queue_draw(); });
		mmy.signal_clicked().connect([this] { --drawingArea.y_; drawingArea.queue_draw(); });
		ppy.signal_clicked().connect([this] { ++drawingArea.y_; drawingArea.queue_draw(); });
		draw.signal_clicked().connect([this] { sfmlArea.queue_draw(); });

		functionQueueDispatcher.connect([this] {
			auto lock = std::unique_lock(functionQueueMutex);
			for (auto fn: functionQueue)
				fn();
			functionQueue.clear();
		});

		add_action("example", Gio::ActionMap::ActivateSlot([this] {
			
		}));

		signal_realize().connect([this] { delay([this] { sfmlArea.init(); }, 2); });
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
			std::unique_lock lock(functionQueueMutex);
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
}
