#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/Server.h"
#include "net/RemoteClient.h"
#include "packet/DragPacket.h"

namespace Game3 {
	void DragPacket::handle(const std::shared_ptr<ServerGame> &, GenericClient &client) {
		if (action != Action::Start && action != Action::Update)
			return;

		auto player = client.getPlayer();
		if (!player)
			return;

		const InventoryPtr inventory = player->getInventory(0);
		if (ItemStackPtr stack = inventory->getActive())
			stack->item->drag(inventory->activeSlot, stack, {position, player->getRealm(), player}, modifiers);
	}
}
