#pragma once

#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct ChatMessageSentPacket: Packet {
		static PacketID ID() { return 47; }

		GlobalID globalID;
		std::string message;

		ChatMessageSentPacket() = default;
		ChatMessageSentPacket(GlobalID global_id, std::string message_):
			globalID(global_id), message(std::move(message_)) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << globalID << message; }
		void decode(Game &, Buffer &buffer)       override { buffer >> globalID >> message; }

		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
