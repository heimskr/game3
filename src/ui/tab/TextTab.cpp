#include <iostream>

#include "game/Game.h"
#include "ui/tab/TextTab.h"

namespace Game3 {
	TextTab::TextTab(Gtk::Notebook &notebook_, const Glib::ustring &text_, const Glib::ustring &name_): Tab(notebook_), text(text_), name(name_) {
		textTagTable = Gtk::TextTagTable::create();
		textBuffer = Gtk::TextBuffer::create(textTagTable);
		textView.set_buffer(textBuffer);
		textView.add_css_class("text-tab");
		textView.set_wrap_mode(Gtk::WrapMode::WORD_CHAR);
		setEditable(false);
		scrolled.set_child(textView);
		scrolled.set_vexpand(true);
	}

	void TextTab::onBlur() {
		if (ephemeral)
			hide();
	}

	void TextTab::update(const std::shared_ptr<ClientGame> &game) {
		Tab::update(game);
	}

	void TextTab::reset(const std::shared_ptr<ClientGame> &game) {
		if (!game) {
			textBuffer->set_text("");
			setName("Text");
			return;
		}

		textBuffer->set_text(text);
		setName(name);
	}

	Glib::ustring TextTab::getText() const {
		return textBuffer->get_text();
	}

	void TextTab::setEditable(bool editable) {
		textView.set_editable(editable);
		textView.set_cursor_visible(editable);
	}
}
