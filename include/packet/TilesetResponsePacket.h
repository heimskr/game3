#pragma once

#include "types/Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct TilesetResponsePacket: Packet {
		static PacketID ID() { return 52; }

		RealmID realmID;
		std::map<TileID, Identifier> map;

		TilesetResponsePacket() = default;
		TilesetResponsePacket(RealmID realm_id, std::map<TileID, Identifier> map_):
			realmID(realm_id), map(std::move(map_)) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << realmID << map; }
		void decode(Game &, BasicBuffer &buffer)  override { buffer >> realmID >> map; }

		void handle(const std::shared_ptr<ClientGame> &) override {}
	};
}
