#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/InteractPacket.h"

namespace Game3 {
	void InteractPacket::handle(ServerGame &game, RemoteClient &client) {
		if (auto player = client.getPlayer()) {
			if (direction && *direction != player->direction) {
				player->teleport(player->position, MovementContext{
					.excludePlayerSelf = true,
					.clearOffset = false,
					.facingDirection = direction
				});
			}

			if (globalID) {
				if (AgentPtr agent = game.getAgent<Agent>(*globalID)) {
					if (direct)
						agent->onInteractOn(player, modifiers);
					else
						agent->onInteractNextTo(player, modifiers);
				} else {
					client.send(ErrorPacket("Can't interact: agent " + std::to_string(*globalID) + " not found"));
				}
			} else if (direct) {
				player->interactOn(modifiers);
			} else {
				player->interactNextTo(modifiers);
			}
		} else {
			WARN("Can't interact: client " << client.id << " has no player");
		}
	}
}
