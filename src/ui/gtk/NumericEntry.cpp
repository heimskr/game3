#include <cassert>
#include <iostream>

#include "ui/gtk/NumericEntry.h"

namespace Game3 {
	static void (*old_insert_text)(GtkEditable *, const char *, int length, int *position) = nullptr;
	static void (*old_activate)(GtkEntry *);
	static bool old_activate_set = false;

	static void new_insert_text(GtkEditable *editable, const char *text, int length, int *position) {
		if (GtkWidget *parent = gtk_widget_get_parent(GTK_WIDGET(editable))) {
			if (auto parent_mm = Glib::wrap(parent, true)) {
				if (auto *numeric_entry = dynamic_cast<NumericEntry *>(parent_mm)) {
					for (auto ch: Glib::ustring(text, length)) {
						if (!Glib::Unicode::isdigit(ch) && ch != '.' && ch != '-')
							return;
						if (ch == '-' && ((position && *position != 0) || (numeric_entry->get_text_length() != 0 && numeric_entry->get_chars(0, 1)[0] == '-')))
							return;
					}
				}
			}
		}

		old_insert_text(editable, text, length, position);
	}

	NumericEntry::NumericEntry() {
		signal_map().connect([this] {
			Gtk::Text *text = dynamic_cast<Gtk::Text *>(get_first_child());
			assert(text != nullptr);
			GtkEditableInterface *interface = GTK_EDITABLE_GET_IFACE(GTK_EDITABLE(text->gobj()));
			if (!old_insert_text)
				old_insert_text = interface->do_insert_text;
			interface->do_insert_text = new_insert_text;

			auto c = GTK_ENTRY_GET_CLASS(GTK_ENTRY(gobj()));

			if (!old_activate_set) {
				old_activate_set = true;
				old_activate = c->activate;
				c->activate = +[](GtkEntry *c_entry) {
					auto *entry = Glib::wrap(c_entry, true);
					if (auto *numeric_entry = dynamic_cast<NumericEntry *>(entry))
						numeric_entry->signal_activate().emit();
					if (old_activate)
						old_activate(c_entry);
				};
			}
		});
	}
}
