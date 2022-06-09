#include "ui/tab/Tab.h"

namespace Game3 {
	class MainWindow;

	class TextTab: public Tab {
		public:
			Glib::ustring text;
			Glib::ustring name;

			TextTab() = delete;
			TextTab(Gtk::Notebook &, const Glib::ustring &text_, const Glib::ustring &name_);

			TextTab(const TextTab &) = delete;
			TextTab(TextTab &&) = delete;

			TextTab & operator=(const TextTab &) = delete;
			TextTab & operator=(TextTab &&) = delete;

			Gtk::Widget & getWidget() override { return scrolled; }
			Glib::ustring getName() override { return name; }
			void update(const std::shared_ptr<Game> &) override;
			void reset(const std::shared_ptr<Game> &) override;

		private:
			Gtk::ScrolledWindow scrolled;
			Gtk::TextView textView;
			Glib::RefPtr<Gtk::TextBuffer> textBuffer;
			Glib::RefPtr<Gtk::TextTagTable> textTagTable;
	};
}
