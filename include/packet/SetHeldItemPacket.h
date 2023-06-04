#pragma once

#include <optional>

#include "game/Game.h"
#include "packet/Packet.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct SetHeldItemPacket: Packet {
		static PacketID ID() { return 35; }

		bool leftHand = false;
		Slot slot = -1;

		SetHeldItemPacket() = default;
		SetHeldItemPacket(bool left_hand, Slot slot_):
			leftHand(left_hand), slot(slot_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << leftHand << slot; }
		void decode(Game &, Buffer &buffer)       override { buffer >> leftHand >> slot; }

		void handle(ServerGame &, RemoteClient &) override;
	};
}
