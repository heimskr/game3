#pragma once

#include "game/Game.h"
#include "packet/Packet.h"

namespace Game3 {
	struct SwapSlotsPacket: Packet {
		static PacketID ID() { return 41; }

		GlobalID agentGID;
		Slot first = -1;
		Slot second = -1;

		SwapSlotsPacket() = default;
		SwapSlotsPacket(GlobalID agent_gid, Slot first_, Slot second_):
			agentGID(agent_gid), first(first_), second(second_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << agentGID << first << second; }
		void decode(Game &, Buffer &buffer)       override { buffer >> agentGID >> first >> second; }

		void handle(ServerGame &, RemoteClient &) override;
	};
}
