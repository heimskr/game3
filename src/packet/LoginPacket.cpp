#include "Log.h"
#include "Tileset.h"
#include "entity/ServerPlayer.h"
#include "game/ServerGame.h"
#include "net/LocalServer.h"
#include "net/RemoteClient.h"
#include "packet/LoginPacket.h"
#include "packet/LoginStatusPacket.h"
#include "packet/RealmNoticePacket.h"
#include "packet/PacketError.h"

namespace Game3 {
	void LoginPacket::handle(ServerGame &game, RemoteClient &client) {
		if (!client.getPlayer()) {
			auto server = game.getServer();
			std::string display_name;
			nlohmann::json json;

			if (game.database.readUser(username, &display_name, &json)) {
				auto player = ServerPlayer::fromJSON(game, json);
				player->username = username;
				client.setPlayer(player);
				{
					auto lock = game.players.uniqueLock();
					game.players.insert(player);
				}
				RealmPtr realm = game.getRealm(player->realmID);
				player->setRealm(realm);
				player->weakClient = client.shared_from_this();
				player->notifyOfRealm(*realm);
				INFO("Player GID is " << player->globalID);
				client.send(LoginStatusPacket(true, player->globalID, username, display_name, player));
				server->setupPlayer(client);
				return;
			}
		}

		client.send(LoginStatusPacket(false));
	}
}
