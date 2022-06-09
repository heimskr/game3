#include "ui/tab/Tab.h"

namespace Game3 {
	void Tab::hide() {
		notebook.remove_page(getWidget());
	}

	void Tab::show() {
		auto &widget = getWidget();
		if (!notebook.get_page(widget))
			notebook.set_current_page(notebook.append_page(widget, getName()));
		else
			notebook.set_current_page(notebook.page_num(widget));
	}

	void Tab::setName(const Glib::ustring &name) {
		if (auto page = notebook.get_page(getWidget()))
			page->property_tab_label().set_value(name);
	}

	void Tab::add() {
		auto &widget = getWidget();
		if (!notebook.get_page(widget))
			notebook.append_page(widget, getName());
	}
}
