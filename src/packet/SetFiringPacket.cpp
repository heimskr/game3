#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/Item.h"
#include "lib/JSON.h"
#include "net/Server.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/SetFiringPacket.h"

namespace Game3 {
	void SetFiringPacket::handle(const std::shared_ptr<ServerGame> &, GenericClient &client) {
		ServerPlayerPtr player = client.getPlayer();
		if (!player) {
			client.sendError("No player.");
			return;
		}

		player->setFiring(firing);
	}
}
