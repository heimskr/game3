#include "algorithm/Stonks.h"
#include "entity/ServerPlayer.h"
#include "error/InsufficientFundsError.h"
#include "game/ServerGame.h"
#include "net/Server.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/DoVillageTradePacket.h"

namespace Game3 {
	void DoVillageTradePacket::handle(ServerGame &game, RemoteClient &client) {
		ServerPlayerPtr player = client.getPlayer();
		if (!player) {
			client.send(ErrorPacket("No player."));
			return;
		}

		VillagePtr village;

		try {
			village = game.getVillage(villageID);
		} catch (const std::out_of_range &) {
			client.send(ErrorPacket("Village " + std::to_string(villageID) + " not found."));
			return;
		}

		if (isSell) {
			try {

			} catch (const InsufficientFundsError &) {
				client.send(ErrorPacket("Village trade failed: insufficient funds."));
				return;
			}

			return;
		}
	}
}
