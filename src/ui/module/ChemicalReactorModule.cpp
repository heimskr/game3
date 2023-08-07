#include "game/ClientGame.h"
#include "tileentity/ChemicalReactor.h"
#include "ui/gtk/UITypes.h"
#include "ui/gtk/Util.h"
#include "ui/module/ChemicalReactorModule.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	ChemicalReactorModule::ChemicalReactorModule(std::shared_ptr<ClientGame> game_, const std::any &argument):
	game(std::move(game_)),
	reactor(std::dynamic_pointer_cast<ChemicalReactor>(std::any_cast<AgentPtr>(argument))) {
		vbox.set_hexpand();
	}

	Gtk::Widget & ChemicalReactorModule::getWidget() {
		return vbox;
	}

	void ChemicalReactorModule::reset() {
		removeChildren(vbox);
		widgets.clear();
		populate();
	}

	void ChemicalReactorModule::update() {
		reset();
	}

	void ChemicalReactorModule::populate() {
		if (!reactor)
			return;

		auto header = std::make_unique<Gtk::Label>("???");
		header->set_text(reactor->getName());
		header->set_margin(10);
		header->set_xalign(0.5);
		vbox.append(*header);
		widgets.push_back(std::move(header));

		entry.set_text(reactor->getEquation());
		// entry.signal_activate().connect([this] {

		// });
	}
}
