#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/SetActiveSlotPacket.h"

namespace Game3 {
	void SetActiveSlotPacket::handle(ServerGame &, RemoteClient &client) {
		if (auto player = client.getPlayer(); player && player->inventory)
			player->inventory->setActive(slot);
	}
}
