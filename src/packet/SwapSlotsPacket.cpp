#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/SwapSlotsPacket.h"

namespace Game3 {
	void SwapSlotsPacket::handle(ServerGame &game, RemoteClient &client) {
		if (firstGID == secondGID && firstSlot == secondSlot)
			return;

		if (!Agent::validateGID(firstGID) || !Agent::validateGID(secondGID)) {
			client.send(ErrorPacket("Can't move slots: invalid GID(s)"));
			return;
		}

		AgentPtr first_agent = game.getAgent(firstGID);
		if (!first_agent) {
			client.send(ErrorPacket("Can't swap slots: first agent not found"));
			return;
		}

		auto first_has_inventory = std::dynamic_pointer_cast<HasInventory>(first_agent);
		if (!first_has_inventory) {
			client.send(ErrorPacket("Can't swap slots: first agent doesn't have an inventory"));
			return;
		}

		if (firstGID == secondGID) {
			first_has_inventory->getInventory()->swap(firstSlot, secondSlot);
		} else {
			AgentPtr second_agent = game.getAgent(secondGID);
			if (!second_agent) {
				client.send(ErrorPacket("Can't swap slots: second agent not found"));
				return;
			}

			auto second_has_inventory = std::dynamic_pointer_cast<HasInventory>(second_agent);
			if (!second_has_inventory) {
				client.send(ErrorPacket("Can't swap slots: second agent doesn't have an inventory"));
				return;
			}

			Inventory &first_inventory  = *first_has_inventory->getInventory();
			Inventory &second_inventory = *second_has_inventory->getInventory();

			{
				auto first_lock  = first_inventory.uniqueLock();
				// Just in case there's some trickery with shared inventories or something.
				auto second_lock = &first_inventory == &second_inventory? std::unique_lock<std::shared_mutex>() : second_inventory.uniqueLock();

				ItemStack *first_stack  = first_inventory[firstSlot];
				ItemStack *second_stack = second_inventory[secondSlot];

				if (first_stack == nullptr && second_stack == nullptr) {
					client.send(ErrorPacket("Can't swap slots: both slots are invalid or empty"));
					return;
				}

				if (first_stack == nullptr) {

					if (!first_inventory.hasSlot(firstSlot)) {
						client.send(ErrorPacket("Can't swap slots: first slot is invalid"));
						return;
					}

					if (first_inventory.onMove)
						first_inventory.onMove(second_inventory, secondSlot, first_inventory, firstSlot, true);

					if (&first_inventory != &second_inventory && second_inventory.onMove)
						second_inventory.onMove(second_inventory, secondSlot, first_inventory, firstSlot, true);

					first_inventory.add(*second_stack, firstSlot);
					second_inventory.erase(secondSlot);

				} else if (second_stack == nullptr) {

					if (!second_inventory.hasSlot(secondSlot)) {
						client.send(ErrorPacket("Can't swap slots: second slot is invalid"));
						return;
					}

					if (first_inventory.onMove)
						first_inventory.onMove(first_inventory, firstSlot, second_inventory, secondSlot, true);

					if (&first_inventory != &second_inventory && second_inventory.onMove)
						second_inventory.onMove(first_inventory, firstSlot, second_inventory, secondSlot, true);

					second_inventory.add(*first_stack, secondSlot);
					first_inventory.erase(firstSlot);

				} else {

					if (first_inventory.onSwap)
						first_inventory.onSwap(first_inventory, firstSlot, second_inventory, secondSlot);

					if (&first_inventory != &second_inventory && second_inventory.onSwap)
						second_inventory.onSwap(second_inventory, secondSlot, first_inventory, firstSlot);

					std::swap(*first_stack, *second_stack);

				}
			}

			first_inventory.notifyOwner();
			second_inventory.notifyOwner();
		}
	}
}
