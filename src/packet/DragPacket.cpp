#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/LocalServer.h"
#include "net/RemoteClient.h"
#include "packet/DragPacket.h"

namespace Game3 {
	void DragPacket::handle(ServerGame &, RemoteClient &client) {
		if (action != Action::Start && action != Action::Update)
			return;

		auto player = client.getPlayer();
		if (!player)
			return;

		const InventoryPtr inventory = player->getInventory();
		if (auto *stack = inventory->getActive())
			stack->item->drag(inventory->activeSlot, *stack, {position, player->getRealm(), player}, modifiers);
	}
}
