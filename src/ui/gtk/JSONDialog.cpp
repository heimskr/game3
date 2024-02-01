#include "ui/gtk/JSONDialog.h"
#include "ui/gtk/NumericEntry.h"
#include "util/Util.h"

namespace Game3 {
	JSONDialog::JSONDialog(Gtk::Window &parent_, const Glib::ustring &title, nlohmann::json specification_):
	Gtk::MessageDialog(parent_, title, false, Gtk::MessageType::OTHER, Gtk::ButtonsType::CANCEL, true),
	specification(std::move(specification_)) {
		auto *area = get_message_area();

		for (const auto &item: specification) {
			const std::string &type = item.at(1);
			const std::string &label = item.at(2);
			const auto &meta = 3 < item.size()? item.at(3) : nlohmann::json();

			if (type == "text" || type == "number") {

				auto label_widget = std::make_shared<Gtk::Label>(label);
				auto entry = type == "text"? std::make_shared<Gtk::Entry>() : std::make_shared<NumericEntry>();
				label_widget->set_halign(Gtk::Align::START);
				if (auto iter = meta.find("initial"); iter != meta.end()) {
					if (iter->is_string())
						entry->set_text(iter->get<std::string>());
					else if (iter->is_number())
						entry->set_text(std::to_string(iter->get<double>()));
					else
						throw std::invalid_argument("Invalid JSON type for initial value of " + label);
				}
				entry->signal_activate().connect(sigc::mem_fun(*this, &JSONDialog::submit));
				area->append(*label_widget);
				area->append(*entry);
				widgets.push_back({label_widget, entry});

			} else if (type == "slider") {

				auto label_widget = std::make_shared<Gtk::Label>(label);
				auto slider = std::make_shared<Gtk::Scale>();
				if (auto iter = meta.find("digits"); iter != meta.end())
					slider->set_digits(iter->get<int>());
				else
					slider->set_digits(2);
				if (auto iter = meta.find("range"); iter != meta.end())
					slider->set_range(iter->at(0).get<double>(), iter->at(1).get<double>());
				if (auto iter = meta.find("noValue"); iter == meta.end() || !iter->get<bool>()) {
					slider->set_value_pos(Gtk::PositionType::RIGHT);
					slider->set_draw_value();
				}
				if (auto iter = meta.find("initial"); iter != meta.end())
					slider->set_value(iter->get<double>());
				if (auto iter = meta.find("increments"); iter != meta.end())
					slider->set_increments(iter->at(0).get<double>(), iter->at(1).get<double>());
				area->append(*label_widget);
				area->append(*slider);
				widgets.push_back({label_widget, slider});

			} else if (type == "bool") {

				// auto label_widget = std::make_shared<Gtk::Label>(label);
				auto check = std::make_shared<Gtk::CheckButton>(label);
				if (auto iter = meta.find("initial"); iter != meta.end())
					check->set_active(iter->get<bool>());
				area->append(*check);
				widgets.push_back({check});

			} else if (type == "ok") {

				add_button(label, Gtk::ResponseType::OK);

			} else
				throw std::invalid_argument("Unsupported widget type in JSONDialog: \"" + type + '"');
		}

		int width, height;
		get_default_size(width, height);
		if (width < 300)
			set_default_size(300, height);
		signal_response().connect([this](int response) {
			if (response == Gtk::ResponseType::OK)
				submit();
			else
				hide();
		});
		set_deletable(false);
	}

	void JSONDialog::submit() {
		hide();

		nlohmann::json data;
		size_t i = 0;

		for (const auto &item: specification) {
			const std::string &name = item.at(0);
			const std::string &type = item.at(1);
			const std::string &label = item.at(2);
			const auto &meta = 3 < item.size()? item.at(3) : nlohmann::json();

			if (type == "text") {
				data[name] = std::dynamic_pointer_cast<Gtk::Entry>(widgets.at(i++).at(1))->get_text();
			} else if (type == "number") {
				try {
					const auto number = parseLong(std::dynamic_pointer_cast<NumericEntry>(widgets.at(i++).at(1))->get_text());
					if (auto iter = meta.find("min"); iter != meta.end() && number < iter->get<long>())
						continue;
					if (auto iter = meta.find("max"); iter != meta.end() && iter->get<long>() < number)
						continue;
					data[name] = number;
				} catch (const std::invalid_argument &) {
					// Intentionally leave out the data.
				}
			} else if (type == "bool") {
				data[name] = std::static_pointer_cast<Gtk::CheckButton>(widgets.at(i++).at(0))->get_active();
			} else if (type == "slider") {
				data[name] = std::dynamic_pointer_cast<Gtk::Scale>(widgets.at(i++).at(1))->get_value();
			}
		}

		signal_submit_.emit(data);
	}
}
