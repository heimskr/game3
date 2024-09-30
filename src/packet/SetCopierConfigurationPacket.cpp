#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/Copier.h"
#include "net/Server.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/SetCopierConfigurationPacket.h"

namespace Game3 {
	void SetCopierConfigurationPacket::handle(const std::shared_ptr<ServerGame> &, RemoteClient &client) {
		ServerPlayerPtr player = client.getPlayer();
		if (!player) {
			client.sendError("No player.");
			return;
		}

		InventoryPtr inventory = player->getInventory(0);
		if (!inventory) {
			client.sendError("Player has no inventory.");
			return;
		}

		ItemStackPtr stack = (*inventory)[slot];
		if (!stack) {
			client.sendError("No copier in slot.");
			return;
		}

		if (!std::dynamic_pointer_cast<Copier>(stack->item)) {
			client.sendError("Slot doesn't contain a copier.");
			return;
		}

		stack->data["includeTileEntities"] = includeTileEntities;
		inventory->notifyOwner({});
	}
}
