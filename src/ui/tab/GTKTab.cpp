#include "ui/tab/GTKTab.h"

namespace Game3 {
	void GTKTab::hide() {
		notebook.remove_page(getWidget());
	}

	void GTKTab::show() {
		Gtk::Widget &widget = getWidget();
		if (!notebook.get_page(widget))
			notebook.set_current_page(notebook.append_page(widget, getName()));
		else
			notebook.set_current_page(notebook.page_num(widget));
	}

	void GTKTab::setName(const Glib::ustring &name) {
		if (auto page = notebook.get_page(getWidget()))
			page->property_tab_label().set_value(name);
	}

	void GTKTab::add() {
		Gtk::Widget &widget = getWidget();
		if (!notebook.get_page(widget))
			notebook.append_page(widget, getName());
	}
}
