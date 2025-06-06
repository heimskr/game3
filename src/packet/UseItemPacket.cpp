#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "net/Server.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/UseItemPacket.h"

namespace Game3 {
	void UseItemPacket::handle(const std::shared_ptr<ServerGame> &, GenericClient &client) {
		ServerPlayerPtr player = client.getPlayer();
		if (!player) {
			client.send(make<ErrorPacket>("No player."));
			return;
		}

		InventoryPtr inventory = player->getInventory(0);
		assert(inventory);

		ItemStackPtr stack = (*inventory)[slot];

		if (!stack) {
			client.send(make<ErrorPacket>("Can't use empty slot."));
			return;
		}

		stack->item->use(slot, stack, player, modifiers);
	}
}
