#include "biology/Gene.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "item/ContainmentOrb.h"
#include "net/Buffer.h"
#include "tileentity/Microscope.h"
#include "ui/gtk/Util.h"
#include "ui/module/InventoryModule.h"
#include "ui/module/MicroscopeModule.h"
#include "ui/tab/InventoryTab.h"
#include "ui/MainWindow.h"

namespace Game3 {
	MicroscopeModule::MicroscopeModule(std::shared_ptr<ClientGame> game_, const std::any &argument):
		MicroscopeModule(game_, std::dynamic_pointer_cast<Microscope>(std::any_cast<AgentPtr>(argument))) {}

	MicroscopeModule::MicroscopeModule(std::shared_ptr<ClientGame> game_, std::shared_ptr<Microscope> microscope_):
	game(std::move(game_)),
	microscope(std::move(microscope_)),
	inventoryModule(std::make_shared<InventoryModule>(game, std::static_pointer_cast<ClientInventory>(microscope->getInventory(0)))) {
		vbox.set_hexpand();

		header.set_text(microscope->getName());
		header.set_margin(10);
		header.set_xalign(0.5);
		vbox.append(header);
		vbox.append(inventoryModule->getWidget());
		vbox.append(geneticAnalysisModule.getWidget());
	}

	Gtk::Widget & MicroscopeModule::getWidget() {
		return vbox;
	}

	void MicroscopeModule::reset() {
		inventoryModule->reset();
		updateResults();
	}

	void MicroscopeModule::update() {
		inventoryModule->update();
		updateResults();
	}

	void MicroscopeModule::updateResults() {
		if (!microscope)
			return;

		auto inventory = microscope->getInventory(0);
		auto lock = inventory->sharedLock();
		geneticAnalysisModule.updateResults((*inventory)[0]);
	}

	void MicroscopeModule::onResize(int width) {
		inventoryModule->onResize(width);
	}

	std::optional<Buffer> MicroscopeModule::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "TileEntityRemoved") {

			if (source && source->getGID() == microscope->getGID()) {
				inventoryModule->handleMessage(source, name, data);
				MainWindow &window = game->getWindow();
				window.queue([&window] { window.removeModule(); });
			}

		} else if (name == "GetAgentGID") {

			return Buffer{microscope->getGID()};

		} else {

			return inventoryModule->handleMessage(source, name, data);

		}

		return std::nullopt;
	}

	void MicroscopeModule::setInventory(std::shared_ptr<ClientInventory> inventory) {
		inventoryModule->setInventory(std::move(inventory));
		updateResults();
	}
}
