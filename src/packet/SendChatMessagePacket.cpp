#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/ServerGame.h"
#include "net/Server.h"
#include "net/RemoteClient.h"
#include "packet/ChatMessageSentPacket.h"
#include "packet/SendChatMessagePacket.h"

namespace Game3 {
	void SendChatMessagePacket::handle(const std::shared_ptr<ServerGame> &game, RemoteClient &client) {
		if (ServerPlayerPtr player = client.getPlayer())
			game->broadcast(ChatMessageSentPacket{player->getGID(), message}, true);
	}
}
