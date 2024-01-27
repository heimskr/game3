#include "Log.h"
#include "error/AuthenticationError.h"
#include "game/ClientGame.h"
#include "net/LocalClient.h"
#include "packet/RegistrationStatusPacket.h"
#include "packet/PacketError.h"
#include "ui/MainWindow.h"

namespace Game3 {
	void RegistrationStatusPacket::handle(ClientGame &game) {
		if (token == 0) {
			game.getWindow().error("Registration failed.");
			return;
		}

		SUCCESS_("Registration succeeded: token = " << token);

		auto &client = *game.client;
		client.setToken(client.getHostname(), username, token);
		client.saveTokens();
	}
}
