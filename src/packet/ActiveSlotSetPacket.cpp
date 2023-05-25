#include "Log.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "net/RemoteClient.h"
#include "packet/ActiveSlotSetPacket.h"

namespace Game3 {
	void ActiveSlotSetPacket::handle(ClientGame &client) {
		client.player->inventory->setActive(slot, true);
	}
}
