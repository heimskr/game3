#include "Log.h"
#include "game/ClientGame.h"
#include "entity/ClientPlayer.h"
#include "packet/SetPlayerStationTypesPacket.h"
#include "ui/MainWindow.h"
#include "ui/tab/CraftingTab.h"
#include "ui/tab/GTKInventoryTab.h"

namespace Game3 {
	void SetPlayerStationTypesPacket::handle(const ClientGamePtr &game) {
		game->getPlayer()->stationTypes = std::move(stationTypes);
		MainWindow &window = game->getWindow();
		window.queue([&window, game, focus = focus] {
			auto tab = window.craftingTab;
			tab->reset(std::static_pointer_cast<ClientGame>(game));
			if (focus)
				tab->show();
			else if (window.getActiveTab() == tab)
				window.inventoryTab->show();
		});
	}
}
