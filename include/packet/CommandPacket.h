#pragma once

#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct CommandPacket: Packet {
		static PacketID ID() { return 6; }

		GlobalID commandID = -1;
		std::string command;

		CommandPacket() = default;
		CommandPacket(GlobalID command_id, std::string command_):
			commandID(command_id), command(std::move(command_)) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << commandID << command; }
		void decode(Game &, Buffer &buffer)       override { buffer >> commandID >> command; }
	};
}
