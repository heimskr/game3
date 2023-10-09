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
		std::optional<Direction> direction;

		InteractPacket() = default;
		InteractPacket(bool direct_, Modifiers modifiers_, std::optional<GlobalID> global_id = {}, std::optional<Direction> direction_ = {}):
			direct(direct_), modifiers(modifiers_), globalID(global_id), direction(direction_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << direct << modifiers << globalID << direction; }
		void decode(Game &, Buffer &buffer)       override { buffer >> direct >> modifiers >> globalID >> direction; }

		void handle(ServerGame &, RemoteClient &) override;
	};
}
