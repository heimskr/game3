#include "Log.h"
#include "game/ClientGame.h"
#include "entity/ClientPlayer.h"
#include "packet/SetPlayerStationTypesPacket.h"
#include "ui/gl/dialog/OmniDialog.h"
#include "ui/gl/tab/CraftingTab.h"
#include "ui/gl/tab/InventoryTab.h"
#include "ui/Window.h"

namespace Game3 {
	void SetPlayerStationTypesPacket::handle(const ClientGamePtr &game) {
		game->getPlayer()->stationTypes = std::move(stationTypes);
		auto window = game->getWindow();
		window->queue([game, focus = focus](Window &window) {
			auto omni = window.getOmniDialog();
			auto tab = omni->craftingTab;
			tab->reset();
			if (focus) {
				omni->activeTab = tab;
				window.showOmniDialog();
			} else if (omni->activeTab == tab) {
				omni->activeTab = omni->inventoryTab;
			}
		});
	}
}
