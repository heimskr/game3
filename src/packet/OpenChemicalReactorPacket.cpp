#include "Log.h"
#include "entity/ClientPlayer.h"
#include "error/PlayerMissingError.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "packet/OpenChemicalReactorPacket.h"
#include "tileentity/ChemicalReactor.h"
#include "ui/MainWindow.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	void OpenChemicalReactorPacket::handle(ClientGame &game) {
		AgentPtr agent = game.getAgent(globalID);
		if (!agent) {
			ERROR("Couldn't find agent " << globalID << " in OpenChemicalReactorPacket handler");
			return;
		}

		auto reactor = std::dynamic_pointer_cast<ChemicalReactor>(agent);
		if (!reactor) {
			ERROR("Couldn't cast agent to ChemicalReactor in OpenChemicalReactorPacket handler");
			return;
		}

		MainWindow &window = game.getWindow();
		std::shared_ptr<InventoryTab> tab = window.inventoryTab;

		game.player->queueForMove([tab](const auto &) {
			tab->removeModule();
			return true;
		});

		tab->show();

		// tab->setModule(...);
	}
}
