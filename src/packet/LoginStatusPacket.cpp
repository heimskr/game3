#include "Log.h"
#include "entity/ClientPlayer.h"
#include "error/AuthenticationError.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "packet/LoginStatusPacket.h"
#include "packet/PacketError.h"
#include "ui/Window.h"

namespace Game3 {
	LoginStatusPacket::LoginStatusPacket(bool success_, GlobalID global_id, std::string_view username_, std::string_view display_name, std::shared_ptr<Player> player):
	success(success_), globalID(global_id), username(username_), displayName(display_name) {
		assert(!success || (!username.empty() && !display_name.empty()));
		if (player) {
			player->encode(playerDataBuffer);
			playerDataBuffer.context = player->getGame();
		}
	}

	void LoginStatusPacket::encode(Game &, Buffer &buffer) const {
		buffer << success << globalID << username << displayName << playerDataBuffer;
	}

	void LoginStatusPacket::decode(Game &game, Buffer &buffer) {
		playerDataBuffer.context = game.shared_from_this();
		buffer >> success >> globalID >> username >> displayName >> playerDataBuffer;
	}

	void LoginStatusPacket::handle(const ClientGamePtr &game) {
		if (!success)
			throw AuthenticationError("Login failed");

		SUCCESS(2, "Login succeeded");
		auto window = game->getWindow();
		{
			auto lock = window->settings.uniqueLock();
			window->settings.username = username;
		}
		window->saveSettings();
		auto player = Entity::create<ClientPlayer>();
		game->setPlayer(player);
		player->setGID(globalID);
		INFO(2, "Setting player GID to {}", globalID);
		player->init(game);
		player->decode(playerDataBuffer);
		player->setupRealm(*game);
		RealmPtr realm = player->getRealm();
		game->setActiveRealm(realm);
		realm->add(player, player->getPosition());
		realm->addPlayer(player);
		player->getInventory(0)->notifyOwner();
	}
}
