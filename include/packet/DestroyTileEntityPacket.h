#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class TileEntity;

	struct DestroyTileEntityPacket: Packet {
		static PacketID ID() { return 27; }

		GlobalID globalID = -1;

		DestroyTileEntityPacket() = default;
		DestroyTileEntityPacket(const TileEntity &);
		DestroyTileEntityPacket(GlobalID global_id):
			globalID(global_id) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << globalID; }
		void decode(Game &, BasicBuffer &buffer)       override { buffer >> globalID; }

		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
