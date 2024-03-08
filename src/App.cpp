#include <iostream>

#include "App.h"
#include "ui/MainWindow.h"

namespace Game3 {
	App::App(): Gtk::Application("gay.heimskr.game3") {
		auto theme = Gtk::IconTheme::get_for_display(Gdk::Display::get_default());
		theme->add_resource_path("/game3/icons");
	}

	Glib::RefPtr<App> App::create() {
		return Glib::make_refptr_for_instance<App>(new App);
	}

	void App::on_startup() {
		Gtk::Application::on_startup();
		set_accel_for_action("win.connect",      "<Ctrl>n");
		set_accel_for_action("win.autoconnect",  "<Ctrl>o");
		set_accel_for_action("win.login",        "<Ctrl>l");
		set_accel_for_action("win.register",     "<Ctrl>r");
		set_accel_for_action("win.debug",        "<Ctrl>d");
		set_accel_for_action("win.settings",     "<Ctrl>h");
		set_accel_for_action("win.play_locally", "<Ctrl>p");
		set_accel_for_action("win.toggle_log",   "<Ctrl><Shift>l");
	}

	void App::on_activate() {
		try {
			auto window = create_window();
			window->present();
		} catch (const Glib::Error &err) {
			std::cerr << "App::on_activate(): " << err.what() << std::endl;
		} catch (const std::exception &err) {
			std::cerr << "App::on_activate(): " << err.what() << std::endl;
		}
	}

	MainWindow * App::create_window() {
		MainWindow *window = MainWindow::create();
		add_window(*window);
		window->signal_hide().connect(sigc::bind(sigc::mem_fun(*this, &App::on_hide_window), window));
		return window;
	}

	const char * App::get_text(const std::string &path, gsize &size) {
		return reinterpret_cast<const char *>(Gio::Resource::lookup_data_global(path)->get_data(size));
	}

	const char * App::get_text(const std::string &path) {
		gsize size;
		return get_text(path, size);
	}

	void App::on_hide_window(Gtk::Window *window) {
		delete window;
	}
}
