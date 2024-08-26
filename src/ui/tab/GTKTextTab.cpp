#include <iostream>

#include "game/Game.h"
#include "ui/tab/GTKTextTab.h"

namespace Game3 {
	GTKTextTab::GTKTextTab(Gtk::Notebook &notebook_, const Glib::ustring &text_, const Glib::ustring &name_): GTKTab(notebook_), text(text_), name(name_) {
		textTagTable = Gtk::TextTagTable::create();
		textBuffer = Gtk::TextBuffer::create(textTagTable);
		textView.set_buffer(textBuffer);
		textView.add_css_class("text-tab");
		textView.set_wrap_mode(Gtk::WrapMode::WORD_CHAR);
		setEditable(false);
		scrolled.set_child(textView);
		scrolled.set_vexpand(true);
	}

	void GTKTextTab::onBlur() {
		if (ephemeral)
			hide();
	}

	void GTKTextTab::update(const std::shared_ptr<ClientGame> &game) {
		GTKTab::update(game);
	}

	void GTKTextTab::reset(const std::shared_ptr<ClientGame> &game) {
		if (!game) {
			textBuffer->set_text("");
			setName("Text");
			return;
		}

		textBuffer->set_text(text);
		setName(name);
	}

	Glib::ustring GTKTextTab::getText() const {
		return textBuffer->get_text();
	}

	void GTKTextTab::setEditable(bool editable) {
		textView.set_editable(editable);
		textView.set_cursor_visible(editable);
	}
}
