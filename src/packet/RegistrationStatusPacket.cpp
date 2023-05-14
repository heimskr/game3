#include "error/AuthenticationError.h"
#include "game/ClientGame.h"
#include "net/LocalClient.h"
#include "packet/RegistrationStatusPacket.h"
#include "packet/PacketError.h"

namespace Game3 {
	void RegistrationStatusPacket::handle(ClientGame &game) {
		if (token == 0)
			throw AuthenticationError("Registration failed");

		std::cout << "Registration succeeded: token = " << token << '\n';

		auto &client = *game.client;
		client.setToken(client.getHostname(), username, token);
		client.saveTokens();
	}
}
