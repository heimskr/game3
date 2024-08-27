#include "Log.h"
#include "error/AuthenticationError.h"
#include "game/ClientGame.h"
#include "net/LocalClient.h"
#include "packet/RegistrationStatusPacket.h"
#include "packet/PacketError.h"
#include "ui/MainWindow.h"

namespace Game3 {
	void RegistrationStatusPacket::handle(const ClientGamePtr &game) {
		if (token == 0) {
			game->getWindow().error("Registration failed.");
			return;
		}

		SUCCESS("Registration succeeded: token = {}", token);

		LocalClientPtr client = game->getClient();
		client->setToken(client->getHostname(), username, token);
		client->saveTokens();
	}
}
