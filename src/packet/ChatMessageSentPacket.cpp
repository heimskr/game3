#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "packet/ChatMessageSentPacket.h"

namespace Game3 {
	void ChatMessageSentPacket::handle(ClientGame &game) {
		auto player = game.getAgent<ClientPlayer>(globalID);
		if (!player) {
			WARN("Couldn't find player with GID " << globalID << " in ChatMessageSentPacket handler");
			return;
		}

		player->setLastMessage(std::move(message));
	}
}
