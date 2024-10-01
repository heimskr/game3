#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class Entity;

	struct KnownItemsPacket: Packet {
		static PacketID ID() { return 64; }

		std::set<Identifier> itemIDs;

		KnownItemsPacket() = default;
		KnownItemsPacket(std::set<Identifier> item_ids):
			itemIDs(std::move(item_ids)) {}
		KnownItemsPacket(const Player &);

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << itemIDs; }
		void decode(Game &, Buffer &buffer)       override { buffer >> itemIDs; }

		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
