#pragma once

#include "Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct InteractPacket: Packet {
		static PacketID ID() { return 21; }

		bool direct = false;
		Modifiers modifiers;

		InteractPacket() = default;
		InteractPacket(bool direct_):
			direct(direct_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << direct << modifiers; }
		void decode(Game &, Buffer &buffer)       override { buffer >> direct >> modifiers; }

		void handle(ServerGame &, RemoteClient &) override;
	};
}
