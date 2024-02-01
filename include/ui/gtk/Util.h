#pragma once

#include <gtkmm.h>

namespace Game3 {
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

	// std::declval<Fn>()(int{}, double{}, double{})

	template <typename T, typename... Args>
	concept CallableWith = requires(T t) {
		t(std::declval<Args>()...);
	};

	template <typename Fn>
	requires CallableWith<Fn, int, double, double>
	Glib::RefPtr<Gtk::GestureClick> createClick(Fn &&fn, int button = -1, bool press = false) {
		auto click = Gtk::GestureClick::create();

		if (press) {
			click->signal_pressed().connect(std::move(fn));
		} else {
			click->signal_released().connect(std::move(fn));
		}

		if (0 <= button)
			click->set_button(button);

		return click;
	}

	template <typename Fn>
	requires (CallableWith<Fn> && !CallableWith<Fn, int, double, double>)
	Glib::RefPtr<Gtk::GestureClick> createClick(Fn &&fn, int button = -1, bool press = false) {
		auto click = Gtk::GestureClick::create();

		auto wrapped = [fn = std::move(fn)](int, double, double) {
			fn();
		};

		if (press) {
			click->signal_pressed().connect(std::move(wrapped));
		} else {
			click->signal_released().connect(std::move(wrapped));
		}

		if (0 <= button)
			click->set_button(button);

		return click;
	}
}
