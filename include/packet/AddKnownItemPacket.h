#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class Entity;

	struct AddKnownItemPacket: Packet {
		static PacketID ID() { return 65; }

		Identifier itemID;

		AddKnownItemPacket() = default;
		AddKnownItemPacket(Identifier item_id);

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << itemID; }
		void decode(Game &, Buffer &buffer)       override { buffer >> itemID; }

		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
