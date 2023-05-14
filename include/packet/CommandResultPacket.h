#pragma once

#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct CommandResultPacket: Packet {
		static PacketID ID() { return 5; }

		GlobalID commandID = -1;
		bool success = false;
		std::string message;

		CommandResultPacket() = default;
		CommandResultPacket(GlobalID command_id, bool success_, std::string message_):
			commandID(command_id), success(success_), message(std::move(message_)) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << commandID << success << message; }
		void decode(Game &, Buffer &buffer)       override { buffer >> commandID >> success >> message; }

		void handle(ClientGame &) override;
	};
}
