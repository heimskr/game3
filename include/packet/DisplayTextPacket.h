#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct DisplayTextPacket: Packet {
		static PacketID ID() { return 48; }

		std::string name;
		std::string message;
		bool removeOnMove = true;

		DisplayTextPacket() = default;
		DisplayTextPacket(std::string name, std::string message, bool remove_on_move):
			name(std::move(name)), message(std::move(message)), removeOnMove(remove_on_move) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << name << message << removeOnMove; }
		void decode(Game &, BasicBuffer &buffer)       override { buffer >> name >> message >> removeOnMove; }

		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
