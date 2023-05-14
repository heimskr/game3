#pragma once

#include "game/ChunkPosition.h"
#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct RealmNoticePacket: Packet {
		static PacketID ID() { return 9; }

		RealmID realmID = -1;
		Identifier type;
		Identifier tileset;
		int64_t seed = 0;
		bool outdoors = false;

		RealmNoticePacket() = default;
		RealmNoticePacket(RealmID realm_id, Identifier type_, Identifier tileset_, int64_t seed_, bool outdoors_):
			realmID(realm_id), type(std::move(type_)), tileset(std::move(tileset_)), seed(seed_), outdoors(outdoors_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << realmID << type << tileset << seed << outdoors; }
		void decode(Game &, Buffer &buffer)       override { buffer >> realmID >> type >> tileset >> seed >> outdoors; }

		void handle(ClientGame &) override;
	};
}
