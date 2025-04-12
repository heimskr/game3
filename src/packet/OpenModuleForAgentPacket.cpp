#include "util/Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "ui/GameUI.h"
#include "ui/Window.h"

namespace Game3 {
	void OpenModuleForAgentPacket::handle(const ClientGamePtr &game) {
		AgentPtr agent = game->getAgent(agentGID);
		if (!agent) {
			ERR("Couldn't find agent {} in OpenModuleForAgentPacket handler", agentGID);
			return;
		}

		auto window = game->getWindow();

		if (removeOnMove) {
			game->getPlayer()->queueForMove([window](const auto &, bool) {
				window->queue([](Window &window) {
					if (auto game_ui = window.uiContext.getUI<GameUI>()) {
						game_ui->removeModule();
						game_ui->hideOmniDialog();
					}
				});
				return true;
			});
		}

		window->queue([agent, module_id = moduleID](Window &window) {
			if (auto game_ui = window.uiContext.getUI<GameUI>()) {
				game_ui->openModule(module_id, std::any(agent));
			}
		});
	}
}
