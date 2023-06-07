#pragma once

#include <memory>
#include <vector>

#include <gtkmm.h>
#include <nlohmann/json.hpp>

namespace Game3 {
	class JSONDialog: public Gtk::MessageDialog {
		public:
			JSONDialog(Gtk::Window &, const Glib::ustring &title, nlohmann::json);

			inline auto signal_submit() const { return signal_submit_; }

		private:
			sigc::signal<void(const nlohmann::json &)> signal_submit_;
			nlohmann::json specification;
			std::vector<std::vector<std::shared_ptr<Gtk::Widget>>> widgets;

			void submit();
	};
}
