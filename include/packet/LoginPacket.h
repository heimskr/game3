#pragma once

#include "game/ChunkPosition.h"
#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct LoginPacket: Packet {
		static PacketID ID() { return 10; }

		std::string username;
		Token token = 0;

		LoginPacket() = default;
		LoginPacket(std::string_view username_, Token token_):
			username(username_), token(token_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << username << token; }
		void decode(Game &, Buffer &buffer)       override { buffer >> username >> token; }

		void handle(ServerGame &, RemoteClient &) const override;
	};
}
