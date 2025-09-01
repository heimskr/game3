#pragma once

#include "game/Game.h"
#include "packet/Packet.h"

namespace Game3 {
	struct DropItemPacket: Packet {
		static PacketID ID() { return 39; }

		Slot slot = -1;
		bool discard = false;

		DropItemPacket() = default;
		DropItemPacket(Slot slot, bool discard):
			slot(slot), discard(discard) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << slot << discard; }
		void decode(Game &, BasicBuffer &buffer)  override { buffer >> slot >> discard; }

		void handle(const std::shared_ptr<ServerGame> &, GenericClient &) override;
	};
}
