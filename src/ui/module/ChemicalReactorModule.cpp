#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "net/Buffer.h"
#include "tileentity/ChemicalReactor.h"
#include "ui/gtk/UITypes.h"
#include "ui/gtk/Util.h"
#include "ui/module/ChemicalReactorModule.h"
#include "ui/module/ExternalInventoryModule.h"
#include "ui/tab/InventoryTab.h"
#include "ui/MainWindow.h"

namespace Game3 {
	ChemicalReactorModule::ChemicalReactorModule(std::shared_ptr<ClientGame> game_, const std::any &argument):
		ChemicalReactorModule(game_, std::dynamic_pointer_cast<ChemicalReactor>(std::any_cast<AgentPtr>(argument))) {}

	ChemicalReactorModule::ChemicalReactorModule(std::shared_ptr<ClientGame> game_, std::shared_ptr<ChemicalReactor> reactor_):
	game(std::move(game_)),
	reactor(std::move(reactor_)),
	inventoryModule(std::make_shared<ExternalInventoryModule>(game, std::static_pointer_cast<ClientInventory>(reactor->getInventory(0)))) {
		assert(reactor);
		vbox.set_hexpand();

		header.set_text(reactor->getName());
		header.set_margin(10);
		header.set_xalign(0.5);
		vbox.append(header);

		entry.set_placeholder_text("Equation");
		entry.set_text(reactor->getEquation());
		entry.set_margin(5);

		entry.signal_activate().connect([this] {
			if (reactor)
				game->player->sendMessage(reactor, "SetEquation", entry.get_text().raw());
		});

		entry.signal_changed().connect([this] {
			entry.remove_css_class("equation_error");
			entry.remove_css_class("equation_success");
		});

		vbox.append(entry);
		vbox.append(inventoryModule->getWidget());
	}

	Gtk::Widget & ChemicalReactorModule::getWidget() {
		return vbox;
	}

	void ChemicalReactorModule::reset() {
		inventoryModule->reset();
	}

	void ChemicalReactorModule::update() {
		inventoryModule->update();
	}

	void ChemicalReactorModule::onResize(int width) {
		inventoryModule->onResize(width);
	}

	std::optional<Buffer> ChemicalReactorModule::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "EquationSet") {

			auto *buffer = std::any_cast<Buffer>(&data);
			assert(buffer != nullptr);
			const bool success = buffer->take<bool>();
			if (success) {
				entry.get_root()->unset_focus();
				entry.remove_css_class("equation_error");
				entry.add_css_class("equation_success");
			} else {
				entry.remove_css_class("equation_success");
				entry.add_css_class("equation_error");
			}

		} else if (name == "TileEntityRemoved") {

			if (source && source->getGID() == reactor->getGID()) {
				inventoryModule->handleMessage(source, name, data);
				MainWindow &window = game->getWindow();
				window.queue([&window] { window.removeModule(); });
			}

		} else if (name == "GetAgentGID") {

			return Buffer{reactor->getGID()};

		} else {
			return inventoryModule->handleMessage(source, name, data);
		}

		return {};
	}

	void ChemicalReactorModule::setInventory(std::shared_ptr<ClientInventory> inventory) {
		inventoryModule->setInventory(std::move(inventory));
	}
}
