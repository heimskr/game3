#include "game/ClientGame.h"
#include "game/Village.h"
#include "ui/gtk/UITypes.h"
#include "ui/gtk/Util.h"
#include "ui/module/VillageTradeModule.h"
#include "ui/tab/InventoryTab.h"
#include "ui/MainWindow.h"
#include "util/Util.h"

#include <format>

namespace Game3 {
	VillageTradeModule::VillageTradeModule(std::shared_ptr<ClientGame> game_, const std::any &argument):
	game(std::move(game_)),
	village(std::any_cast<VillagePtr>(argument)) {
		vbox.set_hexpand();
	}

	Gtk::Widget & VillageTradeModule::getWidget() {
		return vbox;
	}

	void VillageTradeModule::reset() {
		removeChildren(vbox);
		widgets.clear();
		populate();
	}

	void VillageTradeModule::update() {
		reset();
	}

	std::optional<Buffer> VillageTradeModule::handleMessage(const std::shared_ptr<Agent> &, const std::string &name, std::any &data) {
		if (name == "VillageUpdate") {

			VillagePtr updated_village = std::any_cast<VillagePtr>(data);
			if (updated_village == village) {
				INFO("Updating village module");
				update();
			} else {
				INFO("Not updating village module");
			}

		}

		return {};
	}

	void VillageTradeModule::populate() {
		assert(village);

		{
			auto label = std::make_unique<Gtk::Label>(village->getName());
			vbox.append(*label);
			label->set_xalign(0.5);
			label->set_hexpand(true);
			label->set_margin_top(10);
			label->set_margin_bottom(5);
			widgets.push_back(std::move(label));
		}

		for (const auto &[resource, amount]: village->getResources()) {
			auto hbox = std::make_unique<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
			auto item_slot = std::make_unique<ItemSlot>(game, -1, nullptr);
			item_slot->setStack({*game, resource});
			item_slot->set_hexpand(false);
			auto label = std::make_unique<Gtk::Label>(std::format("× {:.2f}", amount));
			hbox->append(*item_slot);
			hbox->append(*label);
			label->set_margin_start(5);
			hbox->set_margin_top(5);
			vbox.append(*hbox);
			widgets.push_back(std::move(label));
			widgets.push_back(std::move(item_slot));
			widgets.push_back(std::move(hbox));
		}
	}
}