#include "entity/Player.h"
#include "error/AuthenticationError.h"
#include "game/ServerGame.h"
#include "net/LocalServer.h"
#include "net/RemoteClient.h"
#include "packet/LoginStatusPacket.h"
#include "packet/PacketError.h"

namespace Game3 {
	LoginStatusPacket::LoginStatusPacket(bool success_, std::string_view display_name, std::shared_ptr<Player> player):
	success(success_), displayName(display_name) {
		assert(!success || !display_name.empty());
		if (player)
			player->encode(playerDataBuffer);
	}

	void LoginStatusPacket::handle(ClientGame &game) const {
		if (!success)
			throw AuthenticationError("Login failed");

		std::cout << "Login succeeded\n";
	}
}
