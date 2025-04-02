#include "util/Log.h"
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
	void MoveSlotsPacket::handle(const std::shared_ptr<ServerGame> &game, GenericClient &client) {
		if (firstGID == secondGID && firstInventory == secondInventory && firstSlot == secondSlot) {
			return;
		}

		if (!Agent::validateGID(firstGID) || !Agent::validateGID(secondGID)) {
			client.send(make<ErrorPacket>("Can't move slots: invalid GID(s)"));
			return;
		}

		AgentPtr first_agent = game->getAgent(firstGID);
		if (!first_agent) {
			client.send(make<ErrorPacket>("Can't move slots: first agent not found (" + std::to_string(firstGID) + ')'));
			return;
		}

		AgentPtr second_agent = game->getAgent(secondGID);
		if (!second_agent) {
			client.send(make<ErrorPacket>("Can't move slots: second agent not found"));
			return;
		}

		auto first_has_inventory = std::dynamic_pointer_cast<HasInventory>(first_agent);
		if (!first_has_inventory) {
			client.send(make<ErrorPacket>("Can't move slots: first agent doesn't have an inventory"));
			return;
		}

		auto second_has_inventory = std::dynamic_pointer_cast<HasInventory>(second_agent);
		if (!second_has_inventory) {
			client.send(make<ErrorPacket>("Can't move slots: second agent doesn't have an inventory"));
			return;
		}

		Inventory &first_inventory  = *first_has_inventory->getInventory(firstInventory);
		Inventory &second_inventory = *second_has_inventory->getInventory(secondInventory);

		auto first_inventoried  = std::dynamic_pointer_cast<InventoriedTileEntity>(first_has_inventory);
		auto second_inventoried = std::dynamic_pointer_cast<InventoriedTileEntity>(second_has_inventory);

		std::function<void()> first_action, second_action;
		std::optional<std::variant<ItemStackPtr, Slot>> first_notify_argument, second_notify_argument;

		{
			auto first_lock  = first_inventory.uniqueLock();
			// Just in case there's some trickery with shared inventories or something.
			auto second_lock = &first_inventory == &second_inventory? std::unique_lock<DefaultMutex>() : second_inventory.uniqueLock();

			ItemStackPtr first_stack  = first_inventory[firstSlot];
			ItemStackPtr second_stack = second_inventory[secondSlot];

			if (first_stack == nullptr) {
				return;
			}

#ifdef CHECK_SLOT_COMPATIBILITY
			if (first_inventoried && !first_inventoried->canExtractItem(Direction::Invalid, firstSlot)) {
				client.send(make<ErrorPacket>("Can't move slots: first slot isn't extractable"));
				return;
			}

			if (second_stack != nullptr && second_inventoried && !second_inventoried->canInsertItem(*second_stack, Direction::Invalid, secondSlot)) {
				client.send(make<ErrorPacket>("Can't move slots: second slot isn't insertable"));
				return;
			}
#endif

			if (secondSlot == Slot(-1)) {

				second_notify_argument = first_stack;

				if (first_inventory == second_inventory) {
					client.send(make<ErrorPacket>("Can't move slot to an indeterminate slot in the same inventory"));
				} else if (ItemStackPtr leftovers = second_inventory.add(first_stack, secondSlot)) {
					*first_stack = std::move(*leftovers);
				} else {
					first_inventory.erase(firstSlot);
				}

			} else if (second_stack != nullptr && first_stack->canMerge(*second_stack)) {

				if (!second_inventory.canInsert(first_stack)) {
					client.send(make<ErrorPacket>("Can't move slots: not enough room in second inventory"));
					return;
				}

				if (first_inventory.onMove)
					first_action = first_inventory.onMove(first_inventory, firstSlot, second_inventory, secondSlot, false);

				if (&first_inventory != &second_inventory && second_inventory.onMove)
					second_action = second_inventory.onMove(first_inventory, firstSlot, second_inventory, secondSlot, false);

				if (first_inventory == second_inventory) {
					const ItemCount max = second_stack->item->maxCount;
					if (first_stack->count + second_stack->count <= max) {
						second_stack->count += first_stack->count;
						first_inventory.erase(firstSlot);
					} else {
						first_stack->count -= max - second_stack->count;
						second_stack->count = max;
						if (first_stack->count == 0)
							first_inventory.erase(firstSlot);
					}
				} else if (ItemStackPtr leftovers = second_inventory.add(first_stack, secondSlot)) {
					*first_stack = std::move(*leftovers);
				} else {
					first_inventory.erase(firstSlot);
				}

			} else if (second_stack == nullptr) {

				if (!second_inventory.hasSlot(secondSlot)) {
					client.send(make<ErrorPacket>("Can't move slots: second slot is invalid"));
					return;
				}

				if (first_inventory.onMove)
					first_action = first_inventory.onMove(first_inventory, firstSlot, second_inventory, secondSlot, true);

				if (&first_inventory != &second_inventory && second_inventory.onMove)
					second_action = second_inventory.onMove(first_inventory, firstSlot, second_inventory, secondSlot, true);

				second_notify_argument = first_stack;

				if (ItemStackPtr leftovers = second_inventory.add(first_stack, secondSlot)) {
					*first_stack = std::move(*leftovers);
				} else {
					first_inventory.erase(firstSlot);
				}

			} else {

				if (first_inventory.onSwap)
					first_action = first_inventory.onSwap(first_inventory, firstSlot, second_inventory, secondSlot);

				if (&first_inventory != &second_inventory && second_inventory.onSwap)
					second_action = second_inventory.onSwap(second_inventory, secondSlot, first_inventory, firstSlot);

				first_notify_argument = second_stack;
				second_notify_argument = first_stack;

				std::swap(*first_stack, *second_stack);

			}
		}

		first_inventory.notifyOwner(std::move(first_notify_argument));
		second_inventory.notifyOwner(std::move(second_notify_argument));

		if (first_action)
			first_action();

		if (second_action)
			second_action();
	}
}
