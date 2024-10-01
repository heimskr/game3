#include "Log.h"
#include "entity/ClientPlayer.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "packet/AddKnownItemPacket.h"
#include "util/Util.h"

namespace Game3 {
	AddKnownItemPacket::AddKnownItemPacket(Identifier item_id):
		itemID(std::move(item_id)) {}

	void AddKnownItemPacket::handle(const ClientGamePtr &game) {
		game->getPlayer()->addKnownItem(itemID);
	}
}
