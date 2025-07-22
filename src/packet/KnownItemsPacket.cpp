#include "util/Log.h"
#include "entity/ClientPlayer.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "packet/KnownItemsPacket.h"
#include "ui/dialog/OmniDialog.h"
#include "ui/tab/CraftingTab.h"
#include "ui/GameUI.h"
#include "ui/Window.h"
#include "util/Util.h"

namespace Game3 {
	KnownItemsPacket::KnownItemsPacket(const Player &player):
		KnownItemsPacket(player.getKnownItems().copyBase()) {}

	void KnownItemsPacket::handle(const ClientGamePtr &game) {
		game->getPlayer()->setKnownItems(itemIDs);
		game->getWindow()->queue([](Window &window) {
			if (auto game_ui = window.uiContext.getUI<GameUI>()) {
				game_ui->getOmniDialog()->craftingTab->reset();
			}
		});
	}
}
