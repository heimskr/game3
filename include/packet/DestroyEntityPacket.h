#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class Entity;

	struct DestroyEntityPacket: Packet {
		static PacketID ID() { return 23; }

		GlobalID globalID = -1;

		DestroyEntityPacket() = default;
		DestroyEntityPacket(const Entity &);
		DestroyEntityPacket(GlobalID global_id):
			globalID(global_id) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << globalID; }
		void decode(Game &, Buffer &buffer)       override { buffer >> globalID; }

		void handle(ClientGame &) override;
	};
}
