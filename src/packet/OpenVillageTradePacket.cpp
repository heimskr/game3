#include "util/Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "packet/OpenVillageTradePacket.h"
#include "types/DirectedPlace.h"
#include "ui/Window.h"
#include "ui/gl/module/VillageTradeModule.h"

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
					window.removeModule();
					window.hideOmniDialog();
				});
				return true;
			});
		}

		// Force a fresh module construction instead of an update
		window->removeModule();

		window->queue([village = std::move(village)](Window &window) {
			window.openModule(VillageTradeModule::ID(), std::any(village));
		});
	}
}
