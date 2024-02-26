#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "packet/ChatMessageSentPacket.h"

namespace Game3 {
	void ChatMessageSentPacket::handle(const ClientGamePtr &game) {
		auto player = game->getAgent<ClientPlayer>(globalID);
		if (!player) {
			WARN_("Couldn't find player with GID " << globalID << " in ChatMessageSentPacket handler");
			return;
		}

		INFO_("[" << player->displayName << "] " << message);
		player->setLastMessage(std::move(message));
	}
}
