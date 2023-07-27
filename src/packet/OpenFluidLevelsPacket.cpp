#include "Log.h"
#include "entity/ClientPlayer.h"
#include "error/PlayerMissingError.h"
#include "game/ClientGame.h"
#include "game/HasFluids.h"
#include "packet/OpenFluidLevelsPacket.h"
#include "ui/MainWindow.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	void OpenFluidLevelsPacket::handle(ClientGame &game) {
		AgentPtr agent = game.getAgent(globalID);
		if (!agent) {
			ERROR("Couldn't find agent " << globalID << " in OpenFluidLevelsPacket handler");
			return;
		}

		auto has_fluids = std::dynamic_pointer_cast<HasFluids>(agent);
		if (!has_fluids) {
			ERROR("Couldn't cast agent to HasFluids in OpenFluidLevelsPacket handler");
			return;
		}

		auto &window = game.getWindow();
		auto tab = window.inventoryTab;

		game.player->queueForMove([tab](const auto &) {
			tab->removeModule();
			return true;
		});

		window.showFluids(has_fluids);
	}
}
