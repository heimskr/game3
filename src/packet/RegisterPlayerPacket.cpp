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
			std::cout << "Failed.\n";
			client.send(RegistrationStatusPacket());
			return;
		}

		auto player = game.server->loadPlayer(username, displayName);
		std::cout << "Succeeded (" << player->token << ").\n";
		client.send(RegistrationStatusPacket(username, displayName, player->token));
	}
}
