#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "ui/MainWindow.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	void OpenModuleForAgentPacket::handle(ClientGame &game) {
		AgentPtr agent = game.getAgent(agentGID);
		if (!agent) {
			ERROR_("Couldn't find agent " << agentGID << " in OpenModuleForAgentPacket handler");
			return;
		}

		MainWindow &window = game.getWindow();

		if (removeOnMove) {
			game.getPlayer()->queueForMove([&window, tab = window.inventoryTab](const auto &, bool) {
				window.queue([tab] {
					tab->removeModule();
				});
				return true;
			});
		}

		window.queue([&window, agent, module_id = moduleID] {
			window.openModule(module_id, std::any(agent));
		});
	}
}
