#pragma once

#include <gtkmm.h>

void setMargins(Gtk::Widget &widget, int margin);

template <typename T>
void removeChildren(T &widget) {
	while (Gtk::Widget *child = widget.get_first_child())
		widget.remove(*child);
}

template <typename T>
void removeChildren(T *widget) {
	while (Gtk::Widget *child = widget->get_first_child())
		widget->remove(*child);
}

template <typename T>
Gtk::TreeViewColumn * appendColumn(Gtk::TreeView &tree_view, const Glib::ustring &title, const Gtk::TreeModelColumn<T> &model_column) {
	Gtk::TreeViewColumn *column = tree_view.get_column(tree_view.append_column(title, model_column) - 1);
	column->set_sort_column(model_column);
	return column;
}

Gtk::TreeViewColumn * appendColumn(Gtk::TreeView &tree_view, const Glib::ustring &title, const Gtk::TreeModelColumn<double> &model_column);
