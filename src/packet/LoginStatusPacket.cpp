#include "util/Log.h"
#include "entity/ClientPlayer.h"
#include "error/AuthenticationError.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "packet/LoginStatusPacket.h"
#include "packet/PacketError.h"
#include "ui/gl/dialog/ConnectionDialog.h"
#include "ui/Window.h"

namespace Game3 {
	LoginStatusPacket::LoginStatusPacket(bool success, GlobalID global_id, std::string username, std::string display_name, std::shared_ptr<Player> player):
	success(success), globalID(global_id), username(std::move(username)), displayName(std::move(display_name)) {
		assert(!success || (!this->username.empty() && !displayName.empty()));
		if (player) {
			player->encode(playerDataBuffer);
			playerDataBuffer.context = player->getGame();
		}
	}

	LoginStatusPacket::LoginStatusPacket(std::string message):
		success(false),
		message(std::move(message)) {}

	void LoginStatusPacket::encode(Game &, Buffer &buffer) const {
		buffer << success << globalID << username << displayName << message << playerDataBuffer;
	}

	void LoginStatusPacket::decode(Game &game, Buffer &buffer) {
		playerDataBuffer.context = game.shared_from_this();
		buffer >> success >> globalID >> username >> displayName >> message >> playerDataBuffer;
	}

	void LoginStatusPacket::handle(const ClientGamePtr &game) {
		auto window = game->getWindow();

		if (!success) {
			window->queue([](Window &window) {
				window.closeGame();
			});
			throw AuthenticationError(message.empty()? "Login failed" : message);
		}

		SUCCESS(2, "Login succeeded");
		{
			auto lock = window->settings.uniqueLock();
			window->settings.username = username;
		}
		window->saveSettings();
		window->uiContext.removeDialogs<ConnectionDialog>();
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
		player->getInventory(0)->notifyOwner({});
	}
}
