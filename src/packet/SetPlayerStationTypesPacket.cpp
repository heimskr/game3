#include "Log.h"
#include "game/ClientGame.h"
#include "entity/ClientPlayer.h"
#include "packet/SetPlayerStationTypesPacket.h"
#include "ui/MainWindow.h"
#include "ui/tab/CraftingTab.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	void SetPlayerStationTypesPacket::handle(ClientGame &game) {
		// TODO!: nanogui
		// game.player->stationTypes = std::move(stationTypes);
		// auto tab = game.getWindow().craftingTab;
		// tab->reset(std::static_pointer_cast<ClientGame>(game.shared_from_this()));
		// if (focus) {
		// 	tab->show();
		// } else {
		// 	MainWindow &window = game.getWindow();
		// 	if (window.getActiveTab() == tab)
		// 		window.inventoryTab->show();
		// }
	}
}
