#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/InteractPacket.h"

namespace Game3 {
	void InteractPacket::handle(ServerGame &, RemoteClient &client) {
		if (auto player = client.getPlayer()) {
			if (direct)
				player->interactOn(modifiers);
			else
				player->interactNextTo(modifiers);
		}
	}
}
