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
			WARN_("No game.");
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
			WARN_("No players.");
		}

		INFO("{} player{}", players.size(), players.size() == 1? ":" : "s:");
		for (const PlayerPtr &player: players) {
			if (auto realm = player->weakRealm.lock()) {
				INFO("- GID[\e[1m{}\e[22m], Position[\e[1m{}\e[22m], RealmID[\e[1m{}\e[22m], Offsets[\e[1m{}\e[22m], Realm->ID[\e[3{}m{}\e[39m]",
					player->getGID(), player->getPosition(), player->realmID, player->offset, player->realmID == realm->id? '2' : '3', realm->id);
				if (!realm->hasEntity(player->getGID()))
					WARN_("  \e[33mSeemingly not present in realm!\e[39m");
				else
					SUCCESS_("  Present in realm.");
			} else
				WARN("- GID[\e[1m{}\e[22m], Position[\e[1m{}\e[22m], RealmID[\e[1m{}\e[22m], Offsets[\e[1m{}\e[22m], no realm pointer", player->getGID(), player->getPosition(), player->realmID, player->offset);
		}

		if (auto player = game->getPlayer())
			INFO("Self ID: {}", player->getGID());
		else
			WARN_("Couldn't find self.");
	}
}
