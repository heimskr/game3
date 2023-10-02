#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/MoveSlotsPacket.h"
#include "tileentity/InventoriedTileEntity.h"

// #define CHECK_SLOT_COMPATIBILITY

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
			client.send(ErrorPacket("Can't move slots: first agent not found (" + std::to_string(firstGID) + ')'));
			return;
		}

		AgentPtr second_agent = game.getAgent(secondGID);
		if (!second_agent) {
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

		Inventory &first_inventory  = *first_has_inventory->getInventory();
		Inventory &second_inventory = *second_has_inventory->getInventory();

		auto first_inventoried  = std::dynamic_pointer_cast<InventoriedTileEntity>(first_has_inventory);
		auto second_inventoried = std::dynamic_pointer_cast<InventoriedTileEntity>(second_has_inventory);

		std::function<void()> first_action, second_action;

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

#ifdef CHECK_SLOT_COMPATIBILITY
			if (first_inventoried && !first_inventoried->canExtractItem(Direction::Invalid, firstSlot)) {
				client.send(ErrorPacket("Can't move slots: first slot isn't extractable"));
				return;
			}

			if (second_stack != nullptr && second_inventoried && !second_inventoried->canInsertItem(*second_stack, Direction::Invalid, secondSlot)) {
				client.send(ErrorPacket("Can't move slots: second slot isn't insertable"));
				return;
			}
#endif

			if (second_stack != nullptr && first_stack->canMerge(*second_stack)) {

				if (!second_inventory.canInsert(*first_stack)) {
					client.send(ErrorPacket("Can't move slots: not enough room in second inventory"));
					return;
				}

				if (first_inventory.onMove)
					first_action = first_inventory.onMove(first_inventory, firstSlot, second_inventory, secondSlot, false);

				if (&first_inventory != &second_inventory && second_inventory.onMove)
					second_action = second_inventory.onMove(first_inventory, firstSlot, second_inventory, secondSlot, false);

				if (std::optional<ItemStack> leftovers = second_inventory.add(*first_stack, secondSlot))
					*first_stack = std::move(*leftovers);
				else
					first_inventory.erase(firstSlot);

			} else if (second_stack == nullptr) {

				if (!second_inventory.hasSlot(secondSlot)) {
					client.send(ErrorPacket("Can't swap slots: second slot is invalid"));
					return;
				}

				if (first_inventory.onMove)
					first_action = first_inventory.onMove(first_inventory, firstSlot, second_inventory, secondSlot, true);

				if (&first_inventory != &second_inventory && second_inventory.onMove)
					second_action = second_inventory.onMove(first_inventory, firstSlot, second_inventory, secondSlot, true);

				second_inventory.add(*first_stack, secondSlot);
				first_inventory.erase(firstSlot);

			} else {

				if (first_inventory.onSwap)
					first_action = first_inventory.onSwap(first_inventory, firstSlot, second_inventory, secondSlot);

				if (&first_inventory != &second_inventory && second_inventory.onSwap)
					second_action = second_inventory.onSwap(second_inventory, secondSlot, first_inventory, firstSlot);

				std::swap(*first_stack, *second_stack);

			}
		}

		first_inventory.notifyOwner();
		second_inventory.notifyOwner();

		if (first_action)
			first_action();

		if (second_action)
			second_action();
	}
}
