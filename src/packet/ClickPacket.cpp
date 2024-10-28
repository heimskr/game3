#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/Server.h"
#include "net/RemoteClient.h"
#include "packet/ClickPacket.h"

namespace Game3 {
	void ClickPacket::handle(const std::shared_ptr<ServerGame> &, GenericClient &client) {
		auto player = client.getPlayer();
		if (!player)
			return;

		const InventoryPtr inventory = player->getInventory(0);

		if (ItemStackPtr stack = inventory->getActive())
			stack->item->use(inventory->activeSlot, stack, Place{position, player->getRealm(), player}, modifiers, {offsetX, offsetY});
	}
}
