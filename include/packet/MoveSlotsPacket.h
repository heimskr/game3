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

		MoveSlotsPacket() = default;
		MoveSlotsPacket(GlobalID first_gid, GlobalID second_gid, Slot first_slot, Slot second_slot):
			firstGID(first_gid), secondGID(second_gid), firstSlot(first_slot), secondSlot(second_slot) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << firstGID << secondGID << firstSlot << secondSlot; }
		void decode(Game &, Buffer &buffer)       override { buffer >> firstGID >> secondGID >> firstSlot >> secondSlot; }

		void handle(ServerGame &, RemoteClient &) override;
	};
}
