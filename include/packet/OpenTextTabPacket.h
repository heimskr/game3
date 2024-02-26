#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct OpenTextTabPacket: Packet {
		static PacketID ID() { return 48; }

		std::string name;
		std::string message;
		bool removeOnMove = true;
		bool ephemeral = true;

		OpenTextTabPacket() = default;
		OpenTextTabPacket(std::string name_, std::string message_, bool remove_on_move, bool ephemeral_):
			name(std::move(name_)), message(std::move(message_)), removeOnMove(remove_on_move), ephemeral(ephemeral_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << name << message << removeOnMove << ephemeral; }
		void decode(Game &, Buffer &buffer)       override { buffer >> name >> message >> removeOnMove >> ephemeral; }

		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
