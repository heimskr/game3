#include "util/Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "packet/OpenVillageTradePacket.h"
#include "types/DirectedPlace.h"
#include "ui/GameUI.h"
#include "ui/Window.h"
#include "ui/module/VillageTradeModule.h"

namespace Game3 {
	void OpenVillageTradePacket::handle(const ClientGamePtr &game) {
		auto window = game->getWindow();

		VillagePtr village;

		try {
			village = game->getVillage(villageID);
		} catch (const std::out_of_range &) {
			WARN("Couldn't find village {}", villageID);
			return;
		}

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

		if (auto game_ui = window->uiContext.getUI<GameUI>()) {
			// Force a fresh module construction instead of an update
			game_ui->removeModule();

			window->queue([village = std::move(village)](Window &window) {
				if (auto game_ui = window.uiContext.getUI<GameUI>()) {
					game_ui->openModule(VillageTradeModule::ID(), std::any(village));
				}
			});
		}
	}
}
