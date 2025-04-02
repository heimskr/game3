#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/FluidGun.h"
#include "item/Item.h"
#include "lib/JSON.h"
#include "net/Server.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/UseFluidGunPacket.h"

namespace Game3 {
	void UseFluidGunPacket::handle(const std::shared_ptr<ServerGame> &, GenericClient &client) {
		ServerPlayerPtr player = client.getPlayer();
		if (!player) {
			return;
		}

		InventoryPtr inventory = player->getInventory(0);

		if (ItemStackPtr stack = inventory->getActive()) {
			if (auto gun = std::dynamic_pointer_cast<FluidGun>(stack->item)) {
				gun->fireGun(inventory->activeSlot, stack, Place{position, player->getRealm(), player}, modifiers, {offsetX, offsetY}, tickFrequency);
			}
		}
	}
}
