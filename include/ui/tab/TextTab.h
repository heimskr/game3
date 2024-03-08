#pragma once

#include "ui/tab/Tab.h"

namespace Game3 {
	class TextTab: public Tab {
		public:
			Glib::ustring text;
			Glib::ustring name;
			/** Whether the tab will disappear on blur. */
			bool ephemeral = false;

			TextTab() = delete;
			TextTab(Gtk::Notebook &, const Glib::ustring &text_, const Glib::ustring &name_);

			Gtk::Widget & getWidget() override { return scrolled; }
			std::string getName() override { return name; }
			void onBlur() override;
			void update(const std::shared_ptr<ClientGame> &) override;
			void reset(const std::shared_ptr<ClientGame> &) override;
			Glib::ustring getText() const;
			void setEditable(bool);

		private:
			Gtk::ScrolledWindow scrolled;
			Gtk::TextView textView;
			Glib::RefPtr<Gtk::TextBuffer> textBuffer;
			Glib::RefPtr<Gtk::TextTagTable> textTagTable;
	};
}
