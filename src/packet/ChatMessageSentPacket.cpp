#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "packet/ChatMessageSentPacket.h"

namespace Game3 {
	void ChatMessageSentPacket::handle(const ClientGamePtr &game) {
		auto player = game->getAgent<ClientPlayer>(globalID);
		if (!player) {
			WARN("Couldn't find player with GID {} in ChatMessageSentPacket handler", globalID);
			return;
		}

		INFO("[{}] {}", player->displayName, message);
		player->setLastMessage(std::move(message));
	}
}
