#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/Server.h"
#include "net/RemoteClient.h"
#include "packet/ClickPacket.h"

namespace Game3 {
	void ClickPacket::handle(ServerGame &, RemoteClient &client) {
		auto player = client.getPlayer();
		if (!player)
			return;

		const InventoryPtr inventory = player->getInventory(0);

		if (auto *stack = inventory->getActive())
			stack->item->use(inventory->activeSlot, *stack, {position, player->getRealm(), player}, modifiers, {offsetX, offsetY}, Hand::None);
	}
}
