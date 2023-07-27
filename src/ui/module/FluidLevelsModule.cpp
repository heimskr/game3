#include "game/ClientGame.h"
#include "game/HasFluids.h"
#include "ui/gtk/UITypes.h"
#include "ui/gtk/Util.h"
#include "ui/module/FluidLevelsModule.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	FluidLevelsModule::FluidLevelsModule(std::shared_ptr<ClientGame> game_, std::shared_ptr<HasFluids> fluid_haver):
	game(std::move(game_)),
	fluidHaver(std::move(fluid_haver)) {
		vbox.set_hexpand();

		// gmenu = Gio::Menu::create();
		// gmenu->append("_Drop", "inventory_popup.drop");
		// gmenu->append("D_iscard", "inventory_popup.discard");

		// popoverMenu.set_parent(vbox);

		// auto source = Gtk::DragSource::create();
		// source->set_actions(Gdk::DragAction::MOVE);
		// source->signal_prepare().connect([this, source](double x, double y) -> Glib::RefPtr<Gdk::ContentProvider> { // Does capturing `source` cause a memory leak?
		// 	auto *item = grid.pick(x, y);

		// 	if (dynamic_cast<Gtk::Fixed *>(item->get_parent()))
		// 		item = item->get_parent();

		// 	if (auto *label = dynamic_cast<Gtk::Label *>(item)) {
		// 		if (label->get_text().empty())
		// 			return nullptr;
		// 	} else if (!dynamic_cast<Gtk::Fixed *>(item))
		// 		return nullptr;

		// 	Glib::Value<DragSource> value;
		// 	value.init(value.value_type());
		// 	value.set({widgetMap.at(item), inventory});
		// 	return Gdk::ContentProvider::create(value);
		// }, false);

		// auto target = Gtk::DropTarget::create(Glib::Value<DragSource>::value_type(), Gdk::DragAction::MOVE);
		// target->signal_drop().connect([this](const Glib::ValueBase &base, double x, double y) {
		// 	if (base.gobj()->g_type != Glib::Value<DragSource>::value_type())
		// 		return false;

		// 	const auto &value = static_cast<const Glib::Value<DragSource> &>(base);
		// 	auto *destination = grid.pick(x, y);

		// 	if (destination != nullptr && destination != &grid) {
		// 		if (dynamic_cast<Gtk::Fixed *>(destination->get_parent()))
		// 			destination = destination->get_parent();

		// 		const DragSource source = value.get();
		// 		game->player->send(MoveSlotsPacket(source.inventory->getOwner()->getGID(), inventory->getOwner()->getGID(), source.slot, widgetMap.at(destination)));
		// 	}

		// 	return true;
		// }, false);

		// grid.add_controller(source);
		// grid.add_controller(target);
	}

	Gtk::Widget & FluidLevelsModule::getWidget() {
		return vbox;
	}

	void FluidLevelsModule::reset() {
		removeChildren(vbox);
		widgets.clear();
		populate();
	}

	void FluidLevelsModule::update() {
		reset();
	}

	// void FluidLevelsModule::onResize(int width) {
	// 	tabWidth = width;
	// 	update();
	// }

	void FluidLevelsModule::populate() {
		if (!fluidHaver)
			return;

		const auto &registry = game->registry<FluidRegistry>();
		auto &levels = fluidHaver->getFluidLevels();
		auto lock = levels.sharedLock();

		auto header = std::make_unique<Gtk::Label>("???");
		if (auto agent = std::dynamic_pointer_cast<Agent>(fluidHaver))
			header->set_text(agent->getName());
		header->set_margin(10);
		header->set_xalign(0.5);
		vbox.append(*header);
		widgets.push_back(std::move(header));

		for (const auto &[id, amount]: levels) {
			const FluidAmount max = fluidHaver->getMaxLevel(id);
			const double progress = amount < max? double(amount) / max : 1.;
			auto hbox  = std::make_unique<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
			std::shared_ptr<Fluid> fluid = registry.maybe(id);
			assert(fluid);
			auto label = std::make_unique<Gtk::Label>(fluid->name);
			auto bar   = std::make_unique<Gtk::ProgressBar>();
			label->set_margin(5);
			bar->set_fraction(progress);
			bar->set_hexpand();
			bar->set_margin_end(5);
			bar->set_valign(Gtk::Align::CENTER);
			hbox->append(*label);
			hbox->append(*bar);
			vbox.append(*hbox);
			widgets.push_back(std::move(hbox));
			widgets.push_back(std::move(label));
			widgets.push_back(std::move(bar));
		}
	}

// 	void FluidLevelsModule::rightClick(Gtk::Widget *widget, int, Slot slot, double x, double y) {
// 		// mainWindow.onBlur();

// 		if (!inventory->contains(slot))
// 			return;

// 		const auto allocation = widget->get_allocation();
// 		x += allocation.get_x();
// 		y += allocation.get_y();

// 		popoverMenu.set_has_arrow(true);
// 		popoverMenu.set_pointing_to({int(x), int(y), 1, 1});
// 		popoverMenu.set_menu_model(gmenu);
// 		lastSlot = slot;
// 		popoverMenu.popup();
// 	}
}
