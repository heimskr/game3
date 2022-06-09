#include <iostream>

#include "game/Game.h"
#include "ui/tab/TextTab.h"

namespace Game3 {
	TextTab::TextTab(Gtk::Notebook &notebook_, const Glib::ustring &text_, const Glib::ustring &name_): Tab(notebook_), text(text_), name(name_) {
		textTagTable = Gtk::TextTagTable::create();
		textBuffer = Gtk::TextBuffer::create(textTagTable);
		textView.set_buffer(textBuffer);
		scrolled.set_child(textView);
		scrolled.set_vexpand(true);
	}

	void TextTab::onBlur() {
		if (ephemeral)
			hide();
	}

	void TextTab::update(const std::shared_ptr<Game> &game) {
		update(game);
	}

	void TextTab::reset(const std::shared_ptr<Game> &game) {
		if (!game)
			return;

		textBuffer->set_text(text);
		setName(name);
	}
}
