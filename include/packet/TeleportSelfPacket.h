#pragma once

#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct TeleportSelfPacket: Packet {
		static PacketID ID() { return 20; }

		RealmID realmID = -1;
		Position position;

		TeleportSelfPacket() = default;
		TeleportSelfPacket(RealmID realm_id, const Position &position_):
			realmID(realm_id), position(position_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << realmID << position; }
		void decode(Game &, Buffer &buffer)       override { buffer >> realmID >> position; }

		void handle(const std::shared_ptr<ServerGame> &, GenericClient &) override;
	};
}
