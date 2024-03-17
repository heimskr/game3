#include "biology/Gene.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "item/ContainmentOrb.h"
#include "net/Buffer.h"
#include "tileentity/Sequencer.h"
#include "ui/gtk/Util.h"
#include "ui/module/SequencerModule.h"
#include "ui/tab/InventoryTab.h"
#include "ui/MainWindow.h"

// document-save-symbolic

namespace Game3 {
	SequencerModule::SequencerModule(std::shared_ptr<ClientGame> game_, const std::any &argument):
		SequencerModule(game_, std::dynamic_pointer_cast<Sequencer>(std::any_cast<AgentPtr>(argument))) {}

	SequencerModule::SequencerModule(std::shared_ptr<ClientGame> game_, std::shared_ptr<Sequencer> sequencer_):
	game(std::move(game_)),
	sequencer(std::move(sequencer_)),
	multiModule(std::make_shared<MultiModule<Substance::Item, Substance::Energy>>(game, std::static_pointer_cast<Agent>(sequencer))) {
		vbox.set_hexpand();

		header.set_text(sequencer->getName());
		header.set_margin(10);
		header.set_xalign(0.5);
		vbox.append(header);
		vbox.append(multiModule->getWidget());
	}

	Gtk::Widget & SequencerModule::getWidget() {
		return vbox;
	}

	void SequencerModule::reset() {
		multiModule->reset();
	}

	void SequencerModule::update() {
		multiModule->update();
	}

	void SequencerModule::onResize(int width) {
		multiModule->onResize(width);
	}

	std::optional<Buffer> SequencerModule::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		assert(sequencer);

		if (name == "TileEntityRemoved") {

			if (source && source->getGID() == sequencer->getGID()) {
				multiModule->handleMessage(source, name, data);
				MainWindow &window = game->getWindow();
				window.queue([&window] { window.removeModule(); });
			}

		} else if (name == "GetAgentGID") {

			return Buffer{sequencer->getGID()};

		} else {

			return multiModule->handleMessage(source, name, data);

		}

		return std::nullopt;
	}

	void SequencerModule::setInventory(std::shared_ptr<ClientInventory> inventory) {
		multiModule->setInventory(std::move(inventory));
	}

	std::shared_ptr<InventoryModule> SequencerModule::getPrimaryInventoryModule() {
		return multiModule->getPrimaryInventoryModule();
	}
}
