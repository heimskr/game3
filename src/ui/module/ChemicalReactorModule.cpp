#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "net/Buffer.h"
#include "tileentity/ChemicalReactor.h"
#include "ui/gtk/UITypes.h"
#include "ui/gtk/Util.h"
#include "ui/module/ChemicalReactorModule.h"
#include "ui/tab/InventoryTab.h"
#include "ui/MainWindow.h"

namespace Game3 {
	ChemicalReactorModule::ChemicalReactorModule(std::shared_ptr<ClientGame> game_, const std::any &argument):
	game(std::move(game_)),
	reactor(std::dynamic_pointer_cast<ChemicalReactor>(std::any_cast<AgentPtr>(argument))) {
		vbox.set_hexpand();

		header.set_text(reactor->getName());
		header.set_margin(10);
		header.set_xalign(0.5);
		vbox.append(header);

		entry.set_text(reactor->getEquation());
		entry.set_margin(5);

		entry.signal_activate().connect([this] {
			if (reactor)
				game->player->sendMessage(*reactor, "SetEquation", entry.get_text().raw());
		});

		entry.signal_changed().connect([this] {
			entry.remove_css_class("equation_error");
			entry.remove_css_class("equation_success");
		});

		vbox.append(entry);
	}

	Gtk::Widget & ChemicalReactorModule::getWidget() {
		return vbox;
	}

	void ChemicalReactorModule::reset() {}

	void ChemicalReactorModule::update() {}

	void ChemicalReactorModule::handleMessage(Agent &source, const std::string &name, Buffer &data) {
		if (name == "EquationSet") {
			const bool success = data.take<bool>();
			if (success) {
				entry.remove_css_class("equation_error");
				entry.add_css_class("equation_success");
			} else {
				entry.remove_css_class("equation_success");
				entry.add_css_class("equation_error");
			}
		} else if (name == "TileEntityRemoved") {
			if (source.getGID() == reactor->getGID()) {
				MainWindow &window = game->getWindow();
				window.queue([&window] { window.removeModule(); });
			}
		}
	}
}
