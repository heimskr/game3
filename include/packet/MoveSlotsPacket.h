#pragma once

#include "game/Game.h"
#include "packet/Packet.h"

namespace Game3 {
	struct MoveSlotsPacket: Packet {
		static PacketID ID() { return 42; }

		GlobalID firstGID  = -1;
		GlobalID secondGID = -1;
		Slot firstSlot  = -1;
		Slot secondSlot = -1;
		InventoryID firstInventory  = 0;
		InventoryID secondInventory = 0;

		MoveSlotsPacket() = default;
		MoveSlotsPacket(GlobalID first_gid, GlobalID second_gid, Slot first_slot, Slot second_slot, InventoryID first_inventory, InventoryID second_inventory):
			firstGID(first_gid), secondGID(second_gid), firstSlot(first_slot), secondSlot(second_slot), firstInventory(first_inventory), secondInventory(second_inventory) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << firstGID << secondGID << firstSlot << secondSlot << firstInventory << secondInventory; }
		void decode(Game &, BasicBuffer &buffer)       override { buffer >> firstGID >> secondGID >> firstSlot >> secondSlot >> firstInventory >> secondInventory; }

		void handle(const std::shared_ptr<ServerGame> &, GenericClient &) override;
	};
}
