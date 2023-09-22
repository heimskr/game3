#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct InteractPacket: Packet {
		static PacketID ID() { return 21; }

		bool direct = false;
		Modifiers modifiers;
		std::optional<GlobalID> globalID;

		InteractPacket() = default;
		InteractPacket(bool direct_, Modifiers modifiers_, std::optional<GlobalID> global_id = {}):
			direct(direct_), modifiers(modifiers_), globalID(global_id) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << direct << modifiers << globalID; }
		void decode(Game &, Buffer &buffer)       override { buffer >> direct >> modifiers >> globalID; }

		void handle(ServerGame &, RemoteClient &) override;
	};
}
