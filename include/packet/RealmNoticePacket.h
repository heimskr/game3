#pragma once

#include "types/ChunkPosition.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class Realm;

	struct RealmNoticePacket: Packet {
		static PacketID ID() { return 9; }

		RealmID realmID = -1;
		Identifier type;
		Identifier tileset;
		int64_t seed = 0;
		bool outdoors = false;

		RealmNoticePacket() = default;
		RealmNoticePacket(Realm &);
		RealmNoticePacket(RealmID realm_id, const Identifier &type_, const Identifier &tileset_, int64_t seed_, bool outdoors_):
			realmID(realm_id), type(type_), tileset(tileset_), seed(seed_), outdoors(outdoors_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << realmID << type << tileset << seed << outdoors; }
		void decode(Game &, BasicBuffer &buffer)  override { buffer >> realmID >> type >> tileset >> seed >> outdoors; }

		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
