#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/InteractPacket.h"

namespace Game3 {
	void InteractPacket::handle(const std::shared_ptr<ServerGame> &game, GenericClient &client) {
		if (ServerPlayerPtr player = client.getPlayer()) {
			if (direction && *direction != player->direction) {
				player->teleport(player->position, MovementContext{
					.excludePlayer = player->getGID(),
					.clearOffset = false,
					.facingDirection = direction
				});
			}

			const ItemStackPtr &held_item = player->getHeld(hand);

			if (globalID) {
				if (AgentPtr agent = game->getAgent<Agent>(*globalID)) {
					if (direct)
						agent->onInteractOn(player, modifiers, held_item, hand);
					else
						agent->onInteractNextTo(player, modifiers, held_item, hand);
				} else {
					client.send(make<ErrorPacket>("Can't interact: agent " + std::to_string(*globalID) + " not found"));
				}
			} else if (direct) {
				player->interactOn(modifiers, held_item, hand);
			} else {
				player->interactNextTo(modifiers, held_item, hand);
			}
		} else {
			WARN("Can't interact: client {} has no player", client.id);
		}
	}
}
