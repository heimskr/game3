#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct SetTileEntityEnergyPacket: Packet {
		static PacketID ID() { return 44; }

		GlobalID agentGID = -1;
		EnergyAmount newEnergy{};

		SetTileEntityEnergyPacket() = default;
		SetTileEntityEnergyPacket(GlobalID agent_gid, EnergyAmount new_energy):
			agentGID(agent_gid), newEnergy(new_energy) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << agentGID << newEnergy; }
		void decode(Game &, Buffer &buffer)       override { buffer >> agentGID >> newEnergy; }

		void handle(ClientGame &) override;
	};
}
