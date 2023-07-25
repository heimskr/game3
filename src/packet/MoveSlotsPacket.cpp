#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/MoveSlotsPacket.h"
#include "tileentity/Chest.h"

namespace Game3 {
	// Magnificent code duplication.
	void MoveSlotsPacket::handle(ServerGame &game, RemoteClient &client) {
		if (firstGID == secondGID && firstSlot == secondSlot)
			return;

		if (!Agent::validateGID(firstGID) || !Agent::validateGID(secondGID)) {
			client.send(ErrorPacket("Can't move slots: invalid GID(s)"));
			return;
		}

		AgentPtr first_agent = game.getAgent(firstGID);
		if (!first_agent) {
			client.send(ErrorPacket("Can't move slots: first agent not found"));
			return;
		}

		AgentPtr second_agent = game.getAgent(secondGID);
		if (!second_agent) {
			WARN("firstGID[" << firstGID << "], secondGID[" << secondGID << "]");
			{
				auto lock = game.allAgents.sharedLock();
				for (const auto &[gid, weak]: game.allAgents) {
					if (auto locked = weak.lock(); std::dynamic_pointer_cast<Chest>(locked)) {
						WARN("Chest: " << gid << " / " << locked->getGID());
					}
				}
			}
			client.send(ErrorPacket("Can't move slots: second agent not found"));
			return;
		}

		auto first_has_inventory = std::dynamic_pointer_cast<HasInventory>(first_agent);
		if (!first_has_inventory) {
			client.send(ErrorPacket("Can't move slots: first agent doesn't have an inventory"));
			return;
		}

		auto second_has_inventory = std::dynamic_pointer_cast<HasInventory>(second_agent);
		if (!second_has_inventory) {
			client.send(ErrorPacket("Can't move slots: second agent doesn't have an inventory"));
			return;
		}

		Inventory &first_inventory  = *first_has_inventory->inventory;
		Inventory &second_inventory = *second_has_inventory->inventory;

		{
			auto first_lock  = first_inventory.uniqueLock();
			// Just in case there's some trickery with shared inventories or something.
			auto second_lock = &first_inventory == &second_inventory? std::unique_lock<std::shared_mutex>() : second_inventory.uniqueLock();

			ItemStack *first_stack  = first_inventory[firstSlot];
			ItemStack *second_stack = second_inventory[secondSlot];

			if (first_stack == nullptr) {
				client.send(ErrorPacket("Can't move slots: first slot is invalid or empty"));
				return;
			}

			if (second_stack != nullptr && first_stack->canMerge(*second_stack)) {
				if (!second_inventory.canInsert(*first_stack)) {
					client.send(ErrorPacket("Can't move slots: not enough room in second inventory"));
					return;
				}

				second_inventory.add(*first_stack, secondSlot);
				first_inventory.erase(firstSlot);
			} else if (second_stack == nullptr) {
				if (!second_inventory.hasSlot(secondSlot)) {
					client.send(ErrorPacket("Can't swap slots: second slot is invalid"));
					return;
				}

				second_inventory.add(*first_stack, secondSlot);
				first_inventory.erase(firstSlot);
			} else {
				std::swap(*first_stack, *second_stack);
			}
		}

		first_inventory.notifyOwner();
		second_inventory.notifyOwner();
	}
}