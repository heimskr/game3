#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class TileEntity;

	struct DestroyTileEntityPacket: Packet {
		static PacketID ID() { return 27; }

		GlobalID globalID = -1;
		RealmID realmID = -1;

		DestroyTileEntityPacket() = default;
		DestroyTileEntityPacket(const TileEntity &);
		DestroyTileEntityPacket(GlobalID global_id, RealmID realm_id):
			globalID(global_id), realmID(realm_id) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << globalID << realmID; }
		void decode(Game &, Buffer &buffer)       override { buffer >> globalID >> realmID; }

		void handle(ClientGame &) override;
	};
}
