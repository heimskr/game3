#include "util/Log.h"
#include "entity/ClientPlayer.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "packet/KnownItemsPacket.h"
#include "ui/gl/dialog/OmniDialog.h"
#include "ui/gl/tab/CraftingTab.h"
#include "ui/Window.h"
#include "util/Util.h"

namespace Game3 {
	KnownItemsPacket::KnownItemsPacket(const Player &player):
		KnownItemsPacket(player.getKnownItems().copyBase()) {}

	void KnownItemsPacket::handle(const ClientGamePtr &game) {
		game->getPlayer()->setKnownItems(itemIDs);
		game->getWindow()->queue([](Window &window) {
			window.getOmniDialog()->craftingTab->reset();
		});
	}
}
