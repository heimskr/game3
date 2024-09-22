#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "ui/Window.h"

namespace Game3 {
	void OpenModuleForAgentPacket::handle(const ClientGamePtr &game) {
		AgentPtr agent = game->getAgent(agentGID);
		if (!agent) {
			ERROR("Couldn't find agent {} in OpenModuleForAgentPacket handler", agentGID);
			return;
		}

		auto window = game->getWindow();

		if (removeOnMove) {
			game->getPlayer()->queueForMove([window](const auto &, bool) {
				window->queue([](Window &window) {
					window.removeModule();
					window.closeOmniDialog();
				});
				return true;
			});
		}

		window->queue([agent, module_id = moduleID](Window &window) {
			window.openModule(module_id, std::any(agent));
		});
	}
}
