#include "Log.h"
#include "command/local/PlayersCommand.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "net/LocalClient.h"
#include "packet/LoginPacket.h"
#include "util/Util.h"

namespace Game3 {
	void PlayersCommand::operator()(LocalClient &client) {
		auto game = client.weakGame.lock();
		if (!game) {
			WARN("No game.");
			return;
		}

		std::vector<PlayerPtr> players;
		{
			auto lock = game->allAgents.sharedLock();
			for (const auto &[gid, weak_agent]: game->allAgents)
				if (AgentPtr agent = weak_agent.lock())
					if (auto player = std::dynamic_pointer_cast<Player>(agent))
						players.push_back(std::move(player));
		}

		if (players.empty()) {
			WARN("No players.");
		}

		INFO(players.size() << " player" << (players.size() == 1? ":" : "s:"));
		for (const PlayerPtr &player: players) {
			if (auto realm = player->weakRealm.lock()) {
				INFO("- GID[\e[1m" << player->getGID() << "\e[22m], Position[\e[1m" << player->getPosition() << "\e[22m], RealmID[\e[1m" << player->realmID << "\e[22m], Realm->ID[\e[3"
					<< (player->realmID == realm->id? '2' : '3') << 'm' << realm->id << "\e[39m]");
				if (!realm->hasEntity(player->getGID()))
					WARN("  \e[33mSeemingly not present in realm!\e[39m");
				else
					SUCCESS("  Present in realm.");
			} else
				WARN("- GID[\e[1m" << player->getGID() << "\e[22m], Position[\e[1m" << player->getPosition() << "\e[22m], RealmID[\e[1m" << player->realmID << "\e[22m], no realm pointer");
		}

		if (game->player)
			INFO("Self ID: " << game->player->getGID());
		else
			WARN("Couldn't find self.");
	}
}
