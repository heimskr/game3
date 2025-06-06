#include "util/Log.h"
#include "graphics/Tileset.h"
#include "entity/ServerPlayer.h"
#include "error/RealmMissingError.h"
#include "game/ServerGame.h"
#include "net/Server.h"
#include "net/RemoteClient.h"
#include "packet/LoginPacket.h"
#include "packet/LoginStatusPacket.h"
#include "packet/RealmNoticePacket.h"
#include "packet/RegistrationStatusPacket.h"
#include "packet/PacketError.h"
#include "realm/Overworld.h"

namespace Game3 {
	void LoginPacket::handle(const std::shared_ptr<ServerGame> &game, GenericClient &client) {
		if (ServerPlayerPtr player = client.getPlayer(); !player) {
			auto server = game->getServer();
			std::string display_name;
			Buffer buffer{game, Side::Server};
			std::optional<Place> release_place;

			GameDB &database = game->getDatabase();

			if (!database.hasUsername(username)) {
				client.send(make<LoginStatusPacket>("User doesn't exist."));
				return;
			}

			const bool was_omnitoken = server->game->compareToken(token);

			if (!was_omnitoken && server->generateToken(username) != token) {
				client.send(make<LoginStatusPacket>("Invalid token."));
				return;
			}

			if (database.readUser(username, &display_name, &buffer, &release_place)) {
				auto player = ServerPlayer::fromBuffer(game, buffer);
				player->username = username;
				client.setPlayer(player);
				game->addPlayer(player);
				RealmPtr realm = game->getRealm(player->realmID);
				player->setRealm(realm);
				player->weakClient = client.weak_from_this();
				player->notifyOfRealm(*realm);
				SUCCESS("Player {} logged in \e[2m(GID {})\e[22m", username, player->globalID);
				player->init(game);
				client.send(make<LoginStatusPacket>(true, player->globalID, username, display_name, player));
				server->setupPlayer(client);
				realm->addPlayer(player);
				if (release_place) {
					player->teleport(release_place->position, release_place->realm, MovementContext{.isTeleport = true});
					database.writeReleasePlace(username, std::nullopt);
				}
				return;
			}

			if (was_omnitoken) {
				if (!displayName || displayName->empty()) {
					client.send(make<LoginStatusPacket>("User doesn't exist and a display name wasn't given."));
					return;
				}

				ServerPlayerPtr player;

				try {
					player = server->loadPlayer(username, *displayName);
				} catch (const RealmMissingError &error) {
					if (error.realmID != 1) {
						throw;
					}

					// Oopsie, we forgot to generate the overworld somehow.
					if (!server->game->initialWorldgen(Overworld::getDefaultSeed())) {
						ERR("Realm 1 is missing but initial worldgen didn't occur.");
						throw;
					}

					// Let's try again.
					player = server->loadPlayer(username, *displayName);
				}

				SUCCESS("Automatically registered user {} with token {}.", username, player->token);
				client.send(make<RegistrationStatusPacket>(username, *displayName, player->token));
				client.setPlayer(player);
				auto realm = player->getRealm();
				player->weakClient = client.weak_from_this();
				player->notifyOfRealm(*realm);
				INFO("Player {}'s GID is {}", username, player->globalID);
				client.send(make<LoginStatusPacket>(true, player->globalID, username, *displayName, player));
				server->setupPlayer(client);
				realm->addPlayer(player);
				return;
			}
		} else {
			WARN("Client already has player. Display name: {}", player->displayName);
		}

		client.send(make<LoginStatusPacket>("Login failed."));
	}
}
