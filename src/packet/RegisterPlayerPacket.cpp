#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/ServerGame.h"
#include "net/LocalServer.h"
#include "net/RemoteClient.h"
#include "packet/RegisterPlayerPacket.h"
#include "packet/RegistrationStatusPacket.h"
#include "packet/LoginStatusPacket.h"
#include "packet/PacketError.h"

namespace Game3 {
	void RegisterPlayerPacket::handle(ServerGame &game, RemoteClient &client) {
		if (game.server->hasUsername(username) || game.server->hasDisplayName(displayName)) {
			WARN("Failed to register user " << username);
			client.send(RegistrationStatusPacket());
			return;
		}

		auto player = game.server->loadPlayer(username, displayName);
		SUCCESS("Registered user " << username << " with token " << player->token << '.');
		client.send(RegistrationStatusPacket(username, displayName, player->token));
		client.setPlayer(player);
		auto realm = player->getRealm();
		player->weakClient = client.shared_from_this();
		player->notifyOfRealm(*realm);
		INFO("Player GID is " << player->globalID);
		client.send(LoginStatusPacket(true, player->globalID, username, displayName, player));
		game.server->setupPlayer(client);
	}
}
