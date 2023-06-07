#pragma once

#include <set>
#include <vector>

#include "game/Game.h"
#include "packet/Packet.h"
#include "ui/Modifiers.h"

// Sorry about the ugly duplication from EntityRequestPacket.

namespace Game3 {
	struct TileEntityRequest {
		GlobalID tileEntityID;
		UpdateCounter threshold;
		explicit TileEntityRequest(TileEntity &);
		TileEntityRequest(GlobalID entity_id, UpdateCounter threshold_):
			tileEntityID(entity_id), threshold(threshold_) {}
	};

	struct TileEntityRequestPacket: Packet {
		static PacketID ID() { return 37; }

		RealmID realmID = -1;
		std::vector<TileEntityRequest> requests;

		TileEntityRequestPacket() = default;
		TileEntityRequestPacket(Realm &, const std::set<ChunkPosition> &);
		TileEntityRequestPacket(RealmID realm_id, std::vector<TileEntityRequest> requests_):
			realmID(realm_id), requests(std::move(requests_)) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override;
		void decode(Game &, Buffer &buffer)       override;

		void handle(ServerGame &, RemoteClient &) override;
	};
}
