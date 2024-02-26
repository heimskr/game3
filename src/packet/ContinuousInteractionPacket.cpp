#include "entity/ServerPlayer.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/ContinuousInteractionPacket.h"
#include "packet/ErrorPacket.h"

namespace Game3 {
	void ContinuousInteractionPacket::handle(const std::shared_ptr<ServerGame> &, RemoteClient &client) {
		if (auto player = client.getPlayer()) {
			if (modifiers) {
				player->continuousInteraction = true;
				player->continuousInteractionModifiers = *modifiers;
			} else
				player->continuousInteraction = false;
			return;
		}

		client.send(ErrorPacket("Can't set continuous interaction: no player"));
	}
}
