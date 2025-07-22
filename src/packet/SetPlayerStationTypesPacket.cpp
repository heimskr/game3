#include "util/Log.h"
#include "game/ClientGame.h"
#include "entity/ClientPlayer.h"
#include "packet/SetPlayerStationTypesPacket.h"
#include "ui/dialog/OmniDialog.h"
#include "ui/tab/CraftingTab.h"
#include "ui/tab/InventoryTab.h"
#include "ui/GameUI.h"
#include "ui/Window.h"

namespace Game3 {
	void SetPlayerStationTypesPacket::handle(const ClientGamePtr &game) {
		game->getPlayer()->stationTypes = std::move(stationTypes);
		auto window = game->getWindow();
		window->queue([game, focus = focus](Window &window) {
			if (auto game_ui = window.uiContext.getUI<GameUI>()) {
				auto omni = game_ui->getOmniDialog();
				auto tab = omni->craftingTab;
				tab->reset();
				if (focus) {
					omni->activeTab = tab;
					game_ui->showOmniDialog();
				} else if (omni->activeTab == tab) {
					omni->activeTab = omni->inventoryTab;
				}
			}
		});
	}
}
