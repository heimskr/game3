#include "entity/ServerPlayer.h"
#include "game/ServerGame.h"
#include "net/Server.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/SubscribeToVillageUpdatesPacket.h"

namespace Game3 {
	void SubscribeToVillageUpdatesPacket::handle(const std::shared_ptr<ServerGame> &game, RemoteClient &client) {
		ServerPlayerPtr player = client.getPlayer();
		if (!player) {
			client.send(make<ErrorPacket>("No player."));
			return;
		}

		if (!villageID) {
			player->unsubscribeVillages();
			return;
		}

		VillagePtr village;

		try {
			village = game->getVillage(*villageID);
		} catch (const std::out_of_range &) {
			client.send(make<ErrorPacket>("Village " + std::to_string(*villageID) + " not found."));
			return;
		}

		player->subscribeVillage(village);
	}
}
