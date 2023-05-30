#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/LocalServer.h"
#include "net/RemoteClient.h"
#include "packet/ClickPacket.h"

namespace Game3 {
	void ClickPacket::handle(ServerGame &game, RemoteClient &client) {
		auto player = client.getPlayer();
		if (!player)
			return;

		if (auto *stack = player->inventory->getActive()) {
			INFO("Click packet: true");
			stack->item->use(player->inventory->activeSlot, *stack, {position, player->getRealm(), player}, {});
		} else
			INFO("Click packet: false");
	}
}
