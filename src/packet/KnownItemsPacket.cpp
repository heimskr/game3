#include "Log.h"
#include "entity/ClientPlayer.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "packet/KnownItemsPacket.h"
#include "util/Util.h"

namespace Game3 {
	KnownItemsPacket::KnownItemsPacket(const Player &player):
		KnownItemsPacket(player.getKnownItems().copyBase()) {}

	void KnownItemsPacket::handle(const ClientGamePtr &game) {
		game->getPlayer()->setKnownItems(itemIDs);
	}
}
