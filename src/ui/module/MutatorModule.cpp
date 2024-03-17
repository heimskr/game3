#include "biology/Gene.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "net/Buffer.h"
#include "tileentity/Mutator.h"
#include "ui/gtk/Util.h"
#include "ui/module/FluidLevelsModule.h"
#include "ui/module/InventoryModule.h"
#include "ui/module/MutatorModule.h"
#include "ui/tab/InventoryTab.h"
#include "ui/MainWindow.h"

namespace Game3 {
	MutatorModule::MutatorModule(std::shared_ptr<ClientGame> game_, const std::any &argument):
		MutatorModule(game_, std::dynamic_pointer_cast<Mutator>(std::any_cast<AgentPtr>(argument))) {}

	MutatorModule::MutatorModule(std::shared_ptr<ClientGame> game_, std::shared_ptr<Mutator> mutator_):
	game(std::move(game_)),
	mutator(std::move(mutator_)),
	inventoryModule(std::make_shared<InventoryModule>(game, std::static_pointer_cast<ClientInventory>(mutator->getInventory(0)))),
	fluidsModule(std::make_shared<FluidLevelsModule>(game, std::make_any<AgentPtr>(mutator), false)),
	geneInfoModule(nullptr) {
		vbox.set_hexpand(true);

		mutateButton.set_hexpand(true);
		mutateButton.set_vexpand(false);
		mutateButton.set_halign(Gtk::Align::CENTER);
		mutateButton.set_valign(Gtk::Align::CENTER);
		mutateButton.signal_clicked().connect([this] {
			mutate();
		});

		header.set_text(mutator->getName());
		header.set_margin(10);
		header.set_xalign(0.5);
		vbox.append(header);
		Gtk::Widget &inventory_widget = inventoryModule->getWidget();
		inventory_widget.set_hexpand(false);
		hbox.append(inventory_widget);
		hbox.append(mutateButton);
		vbox.append(hbox);
		vbox.append(fluidsModule->getWidget());
		vbox.append(geneInfoModule.getWidget());
	}

	Gtk::Widget & MutatorModule::getWidget() {
		return vbox;
	}

	void MutatorModule::reset() {
		inventoryModule->reset();
		fluidsModule->reset();
		geneInfoModule.reset();
	}

	void MutatorModule::update() {
		inventoryModule->update();
		fluidsModule->update();
		assert(mutator);
		geneInfoModule.update(std::shared_ptr<Gene>(mutator->getGene()));
	}

	void MutatorModule::onResize(int width) {
		inventoryModule->onResize(width);
		fluidsModule->onResize(width);
	}

	void MutatorModule::mutate() {
		assert(mutator);
		std::any buffer = std::make_any<Buffer>();
		game->getPlayer()->sendMessage(mutator, "Mutate", buffer);
	}

	std::optional<Buffer> MutatorModule::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "TileEntityRemoved") {

			if (source && source->getGID() == mutator->getGID()) {
				MainWindow &window = game->getWindow();
				window.queue([&window] { window.removeModule(); });
			}

		} else if (name == "GetAgentGID") {

			return Buffer{mutator->getGID()};

		} else if (std::optional<Buffer> buffer = inventoryModule->handleMessage(source, name, data)) {

			return buffer;

		} else if (std::optional<Buffer> buffer = fluidsModule->handleMessage(source, name, data)) {

			return buffer;

		}

		return std::nullopt;
	}

	void MutatorModule::setInventory(std::shared_ptr<ClientInventory> inventory) {
		inventoryModule->setInventory(std::move(inventory));
		if (std::unique_ptr<Gene> gene = mutator->getGene())
			geneInfoModule.update(std::shared_ptr(std::move(gene)));
		else
			geneInfoModule.update(nullptr);
	}
}
