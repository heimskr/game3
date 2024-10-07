#pragma once

#include "types/Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct TilesetRequestPacket: Packet {
		static PacketID ID() { return 51; }

		RealmID realmID;

		TilesetRequestPacket() = default;
		TilesetRequestPacket(RealmID realm_id):
			realmID(realm_id) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << realmID; }
		void decode(Game &, Buffer &buffer)       override { buffer >> realmID; }

		void handle(const std::shared_ptr<ServerGame> &, GenericClient &) override;
	};
}
