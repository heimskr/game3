#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/SwapSlotsPacket.h"

namespace Game3 {
	void SwapSlotsPacket::handle(ServerGame &game, RemoteClient &client) {
		AgentPtr agent = game.getAgent(agentGID);
		if (!agent) {
			client.send(ErrorPacket("Can't swap slots: agent not found"));
			return;
		}

		auto has_inventory = std::dynamic_pointer_cast<HasInventory>(agent);
		if (!has_inventory) {
			client.send(ErrorPacket("Can't swap slots: agent doesn't have an inventory"));
			return;
		}

		has_inventory->inventory->swap(first, second);
	}
}
