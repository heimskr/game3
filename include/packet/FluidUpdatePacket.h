#pragma once

#include "Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct FluidUpdatePacket: Packet {
		static PacketID ID() { return 33; }

		RealmID realmID;
		Position position;
		FluidTile fluidTile;

		FluidUpdatePacket() = default;
		FluidUpdatePacket(RealmID realm_id, const Position &position_, FluidTile fluid_tile):
			realmID(realm_id), position(position_), fluidTile(fluid_tile) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << realmID << position << fluidTile; }
		void decode(Game &, Buffer &buffer)       override { buffer >> realmID >> position >> fluidTile; }

		void handle(ClientGame &) override;
	};
}
