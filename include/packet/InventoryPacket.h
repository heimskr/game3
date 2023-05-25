#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class Inventory;

	struct InventoryPacket: Packet {
		static PacketID ID() { return 24; }

		std::shared_ptr<Inventory> inventory;

		InventoryPacket() = default;
		InventoryPacket(const std::shared_ptr<Inventory> &inventory_):
			inventory(inventory_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override;
		void decode(Game &, Buffer &buffer)       override;

		void handle(ClientGame &) override;
	};
}
