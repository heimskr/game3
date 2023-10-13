#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/ServerGame.h"
#include "net/Server.h"
#include "net/RemoteClient.h"
#include "packet/RegisterPlayerPacket.h"
#include "packet/RegistrationStatusPacket.h"
#include "packet/LoginStatusPacket.h"
#include "packet/PacketError.h"

namespace Game3 {
	void RegisterPlayerPacket::handle(ServerGame &game, RemoteClient &client) {
		auto server = game.getServer();

		if (username.empty() || displayName.empty()) {
			client.send(RegistrationStatusPacket());
			return;
		}

		if (game.database.hasName(username, displayName)) {
			WARN("Failed to register user " << username);
			client.send(RegistrationStatusPacket());
			return;
		}

		auto player = server->loadPlayer(username, displayName);
		SUCCESS("Registered user " << username << " with token " << player->token << '.');
		client.send(RegistrationStatusPacket(username, displayName, player->token));
		client.setPlayer(player);
		auto realm = player->getRealm();
		player->weakClient = client.shared_from_this();
		player->notifyOfRealm(*realm);
		INFO("Player GID is " << player->globalID);
		client.send(LoginStatusPacket(true, player->globalID, username, displayName, player));
		server->setupPlayer(client);
		realm->addPlayer(player);
	}
}
