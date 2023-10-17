#include "Log.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "item/Item.h"
#include "tileentity/Pipe.h"
#include "types/DirectedPlace.h"
#include "ui/gtk/UITypes.h"
#include "ui/module/ItemFilterModule.h"

namespace Game3 {
	ItemFilterModule::ItemFilterModule(std::shared_ptr<ClientGame> game_, const std::any &argument):
	game(std::move(game_)),
	place(std::any_cast<DirectedPlace>(argument)) {
		populate();
	}

	Gtk::Widget & ItemFilterModule::getWidget() {
		return vbox;
	}

	void ItemFilterModule::update() {
		populate();
	}

	void ItemFilterModule::reset() {
		populate();
	}

	std::optional<Buffer> ItemFilterModule::handleMessage(const std::shared_ptr<Agent> &, const std::string &, std::any &) {
		return std::nullopt;
	}

	void ItemFilterModule::populate() {
		removeChildren(vbox);
		widgets.clear();

		auto pipe = std::dynamic_pointer_cast<Pipe>(place.getTileEntity());
		if (!pipe)
			return;

		auto fixed = std::make_unique<Gtk::Fixed>();
		fixed->set_size_request(68, 68);
		fixed->set_halign(Gtk::Align::CENTER);
		fixed->add_css_class("item-slot");

		auto target = Gtk::DropTarget::create(Glib::Value<DragSource>::value_type(), Gdk::DragAction::MOVE);
		target->signal_drop().connect([this](const Glib::ValueBase &base, double, double) {
			if (!filter || base.gobj()->g_type != Glib::Value<DragSource>::value_type())
				return false;

			const DragSource source = static_cast<const Glib::Value<DragSource> &>(base).get();

			ItemStack *stack = nullptr;
			{
				auto lock = source.inventory->sharedLock();
				stack = (*source.inventory)[source.slot];
			}

			if (stack) {
				filter->addItem(*stack);
				populate();
			}

			return true;
		}, false);

		fixed->add_controller(target);

		vbox.append(*fixed);
		widgets.push_back(std::move(fixed));

		filter = &pipe->itemFilters[place.direction];

		std::shared_lock<DefaultMutex> data_lock;
		auto &data = filter->getData(data_lock);
		for (const auto &[id, set]: data) {
			for (const nlohmann::json &json: set) {
				auto hbox = std::make_unique<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
				ItemStack stack(*game, id, 1, json);

				auto image_ptr = std::make_unique<Gtk::Image>(stack.getImage(*game));
				image_ptr->set_margin(10);
				image_ptr->set_margin_top(6);
				image_ptr->set_size_request(32, 32);

				auto entry = std::make_unique<Gtk::Entry>();
				if (std::string text = json.dump(); text != "null")
					entry->set_text(text);
				entry->set_hexpand(true);
				entry->set_margin_end(10);

				hbox->set_margin_top(10);
				hbox->append(*image_ptr);
				hbox->append(*entry);
				vbox.append(*hbox);
				widgets.push_back(std::move(entry));
				widgets.push_back(std::move(image_ptr));
				widgets.push_back(std::move(hbox));
			}
		}
	}
}
