#include "entity/Player.h"
#include "error/AuthenticationError.h"
#include "game/ServerGame.h"
#include "net/LocalServer.h"
#include "net/RemoteClient.h"
#include "packet/RegistrationStatusPacket.h"
#include "packet/PacketError.h"

namespace Game3 {
	void RegistrationStatusPacket::handle(ClientGame &game) const {
		if (token == 0)
			throw AuthenticationError("Registration failed");

		std::cout << "Registration succeeded\n";
	}
}
