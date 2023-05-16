#include "Log.h"
#include "entity/Player.h"
#include "error/AuthenticationError.h"
#include "game/ClientGame.h"
#include "net/LocalServer.h"
#include "net/RemoteClient.h"
#include "packet/LoginStatusPacket.h"
#include "packet/PacketError.h"

namespace Game3 {
	LoginStatusPacket::LoginStatusPacket(bool success_, std::string_view username_, std::string_view display_name, std::shared_ptr<Player> player):
	success(success_), username(username_), displayName(display_name) {
		assert(!success || (!username.empty() && !display_name.empty()));
		if (player)
			player->encode(playerDataBuffer);
	}

	void LoginStatusPacket::handle(ClientGame &game) {
		if (!success)
			throw AuthenticationError("Login failed");

		SUCCESS("Login succeeded");
		game.player = Entity::create<Player>();
		game.player->init(game);
		game.player->decode(playerDataBuffer);
		game.player->setupRealm(game);
		game.activeRealm = game.player->getRealm();
		game.activeRealm->addPlayer(game.player);
	}
}
