#include "util/Log.h"
#include "entity/ServerPlayer.h"
#include "game/ServerGame.h"
#include "net/Server.h"
#include "net/RemoteClient.h"
#include "packet/RegisterPlayerPacket.h"
#include "packet/RegistrationStatusPacket.h"
#include "packet/LoginStatusPacket.h"
#include "packet/PacketError.h"

namespace Game3 {
	void RegisterPlayerPacket::handle(const std::shared_ptr<ServerGame> &game, GenericClient &client) {
		auto server = game->getServer();

		if (username.empty() || displayName.empty()) {
			client.send(make<RegistrationStatusPacket>());
			return;
		}

		if (game->getDatabase().hasUsername(username) || game->getDatabase().hasDisplayName(displayName)) {
			WARN("Failed to register user {}", username);
			client.send(make<RegistrationStatusPacket>());
			return;
		}

		auto player = server->loadPlayer(username, displayName);
		SUCCESS("Registered user {} with token {}.", username, player->token);
		client.send(make<RegistrationStatusPacket>(username, displayName, player->token));
		client.setPlayer(player);
		auto realm = player->getRealm();
		player->weakClient = client.weak_from_this();
		player->notifyOfRealm(*realm);
		INFO("Player GID is {}", player->globalID);
		client.send(make<LoginStatusPacket>(true, player->globalID, username, displayName, player));
		server->setupPlayer(client);
		realm->addPlayer(player);
	}
}
