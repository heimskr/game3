#pragma once

#include "types/Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct TileUpdatePacket: Packet {
		static PacketID ID() { return 4; }

		RealmID realmID;
		Layer layer;
		Position position;
		TileID tileID;

		TileUpdatePacket() = default;
		TileUpdatePacket(RealmID realm_id, Layer layer_, const Position &position_, TileID tile_id):
			realmID(realm_id), layer(layer_), position(position_), tileID(tile_id) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << realmID << layer << position << tileID; }
		void decode(Game &, Buffer &buffer)       override { buffer >> realmID >> layer >> position >> tileID; }

		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
