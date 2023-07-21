#include "Log.h"
#include "entity/ClientPlayer.h"
#include "error/PlayerMissingError.h"
#include "game/ClientGame.h"
#include "packet/OpenAgentInventoryPacket.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	void OpenAgentInventoryPacket::handle(ClientGame &game) {
		AgentPtr agent = game.getAgent(globalID);
		if (!agent) {
			ERROR("Couldn't find agent " << globalID << " in OpenAgentInventoryPacket handler");
			return;
		}

		auto has_inventory = std::dynamic_pointer_cast<HasInventory>(agent);
		if (!has_inventory) {
			ERROR("Couldn't cast agent to HasInventory in OpenAgentInventoryPacket handler");
			return;
		}

		auto tab = game.canvas.window.inventoryTab;

		game.player->queueForMove([player = game.player, tab](const auto &) {
			tab->resetExternalInventory();
			return true;
		});

		tab->setExternalInventory(agent->getName(), has_inventory->inventory, agent);
	}
}
