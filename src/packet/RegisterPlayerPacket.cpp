#include "Log.h"
#include "game/ServerGame.h"
#include "net/LocalServer.h"
#include "net/RemoteClient.h"
#include "packet/RegisterPlayerPacket.h"
#include "packet/RegistrationStatusPacket.h"
#include "packet/LoginStatusPacket.h"
#include "packet/PacketError.h"

namespace Game3 {
	void RegisterPlayerPacket::handle(ServerGame &game, RemoteClient &client) const {
		if (game.server->hasUsername(username) || game.server->hasDisplayName(displayName)) {
			WARN("Failed to register user " << username);
			client.send(RegistrationStatusPacket());
			return;
		}

		auto player = game.server->loadPlayer(username, displayName);
		SUCCESS("Registered user " << username << " with token " << player->token << '.');
		client.send(RegistrationStatusPacket(username, displayName, player->token));
	}
}
