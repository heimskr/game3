#include "Log.h"
#include "entity/ClientPlayer.h"
#include "error/AuthenticationError.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
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
		buffer << success << globalID << username << displayName << playerDataBuffer;
	}

	void LoginStatusPacket::decode(Game &game, Buffer &buffer) {
		playerDataBuffer.context = game.shared_from_this();
		buffer >> success >> globalID >> username >> displayName >> playerDataBuffer;
	}

	void LoginStatusPacket::handle(ClientGame &game) {
		if (!success)
			throw AuthenticationError("Login failed");

		SUCCESS("Login succeeded");
		game.player = Entity::create<ClientPlayer>();
		game.player->setGID(globalID);
		INFO("Setting player GID to " << globalID);
		game.player->init(game);
		game.player->decode(playerDataBuffer);
		game.player->setupRealm(game);
		{
			auto lock = game.activeRealm.uniqueLock();
			game.activeRealm = game.player->getRealm();
			game.activeRealm->add(game.player, game.player->getPosition());
			game.activeRealm->addPlayer(game.player);
		}
		game.player->inventory->notifyOwner();
	}
}
