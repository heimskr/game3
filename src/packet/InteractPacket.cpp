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

			ItemStack *held_item = player->getHeld(hand);

			if (globalID) {
				if (AgentPtr agent = game.getAgent<Agent>(*globalID)) {
					if (direct)
						agent->onInteractOn(player, modifiers, held_item);
					else
						agent->onInteractNextTo(player, modifiers, held_item);
				} else {
					client.send(ErrorPacket("Can't interact: agent " + std::to_string(*globalID) + " not found"));
				}
			} else if (direct) {
				player->interactOn(modifiers, held_item);
			} else {
				player->interactNextTo(modifiers, held_item);
			}
		} else {
			WARN("Can't interact: client " << client.id << " has no player");
		}
	}
}
