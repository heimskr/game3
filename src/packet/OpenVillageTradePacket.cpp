#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "packet/OpenVillageTradePacket.h"
#include "types/DirectedPlace.h"
#include "ui/MainWindow.h"
#include "ui/module/GTKVillageTradeModule.h"
#include "ui/tab/GTKInventoryTab.h"

namespace Game3 {
	void OpenVillageTradePacket::handle(const ClientGamePtr &game) {
		MainWindow &window = game->getWindow();

		VillagePtr village;

		try {
			village = game->getVillage(villageID);
		} catch (const std::out_of_range &) {
			WARN("Couldn't find village {}", villageID);
			return;
		}

		if (removeOnMove) {
			game->getPlayer()->queueForMove([&window, tab = window.inventoryTab](const auto &, bool) {
				window.queue([tab] {
					tab->removeModule();
				});
				return true;
			});
		}

		// Force a fresh module construction instead of an update
		window.inventoryTab->removeModule();

		window.queue([&window, village = std::move(village)] {
			window.openModule(GTKVillageTradeModule::ID(), std::any(village));
		});
	}
}
