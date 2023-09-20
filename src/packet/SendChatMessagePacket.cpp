#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/ServerGame.h"
#include "net/LocalServer.h"
#include "net/RemoteClient.h"
#include "packet/ChatMessageSentPacket.h"
#include "packet/SendChatMessagePacket.h"

namespace Game3 {
	void SendChatMessagePacket::handle(ServerGame &game, RemoteClient &client) {
		game.broadcast(ChatMessageSentPacket{client.getPlayer()->getGID(), message});
	}
}
