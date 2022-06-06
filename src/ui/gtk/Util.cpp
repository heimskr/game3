#include "ui/gtk/Util.h"

void setMargins(Gtk::Widget &widget, int margin) {
	widget.set_margin_top(margin);
	widget.set_margin_end(margin);
	widget.set_margin_bottom(margin);
	widget.set_margin_start(margin);
}

Gtk::TreeViewColumn * appendColumn(Gtk::TreeView &tree_view, const Glib::ustring &title, const Gtk::TreeModelColumn<double> &model_column) {
	Gtk::TreeViewColumn *column = tree_view.get_column(tree_view.append_column_numeric(title, model_column, "%g") - 1);
	column->set_sort_column(model_column);
	return column;
}
