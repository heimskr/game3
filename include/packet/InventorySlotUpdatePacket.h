#pragma once

#include "Position.h"
#include "item/Item.h"
#include "net/Buffer.h"
#include "packet/Packet.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct InventorySlotUpdatePacket: Packet {
		static PacketID ID() { return 22; }

		Slot slot = -1;
		ItemStack stack;

		InventorySlotUpdatePacket() = default;
		InventorySlotUpdatePacket(Slot, const ItemStack &);

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << slot << stack; }
		void decode(Game &, Buffer &buffer)       override { buffer >> slot >> stack; }

		void handle(ClientGame &) override;
	};
}
