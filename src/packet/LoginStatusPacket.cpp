#include "Log.h"
#include "entity/Player.h"
#include "error/AuthenticationError.h"
#include "game/ClientGame.h"
#include "net/LocalServer.h"
#include "net/RemoteClient.h"
#include "packet/LoginStatusPacket.h"
#include "packet/PacketError.h"

namespace Game3 {
	LoginStatusPacket::LoginStatusPacket(bool success_, GlobalID global_id, std::string_view username_, std::string_view display_name, std::shared_ptr<Player> player):
	success(success_), globalID(global_id), username(username_), displayName(display_name) {
		assert(!success || (!username.empty() && !display_name.empty()));
		if (player) {
			player->encode(playerDataBuffer);
			playerDataBuffer.context = player->getGame().shared_from_this();
		}
	}

	void LoginStatusPacket::encode(Game &, Buffer &buffer) const {
		buffer << success << username << displayName << playerDataBuffer;
	}

	void LoginStatusPacket::decode(Game &game, Buffer &buffer) {
		playerDataBuffer.context = game.shared_from_this();
		buffer >> success >> username >> displayName >> playerDataBuffer;
	}

	void LoginStatusPacket::handle(ClientGame &game) {
		if (!success)
			throw AuthenticationError("Login failed");

		SUCCESS("Login succeeded");
		game.player = Entity::create<Player>();
		INFO("[1] game.player.use_count = " << game.player.use_count());
		game.player->init(game);
		INFO("[2] game.player.use_count = " << game.player.use_count());
		INFO("Setting player GID to " << globalID);
		game.player->setGID(globalID);
		INFO("[3] game.player.use_count = " << game.player.use_count());
		game.player->decode(playerDataBuffer);
		INFO("[4] game.player.use_count = " << game.player.use_count());
		game.player->setupRealm(game);
		INFO("[5] game.player.use_count = " << game.player.use_count());
		game.activeRealm = game.player->getRealm();
		INFO("[6] game.player.use_count = " << game.player.use_count());
		game.activeRealm->add(game.player);
		INFO("[7] game.player.use_count = " << game.player.use_count());
		game.activeRealm->addPlayer(game.player);
		INFO("[8] game.player.use_count = " << game.player.use_count());
	}
}
