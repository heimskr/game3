#include "Log.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/InteractPacket.h"

namespace Game3 {
	void InteractPacket::handle(ServerGame &, RemoteClient &client) {
		if (auto player = client.getPlayer()) {
			if (direct)
				player->interactOn();
			else
				player->interactNextTo(modifiers);
		}
	}
}
