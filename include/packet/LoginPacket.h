#pragma once

#include "types/ChunkPosition.h"
#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct LoginPacket: Packet {
		static PacketID ID() { return 10; }

		std::string username;
		Token token{};
		std::optional<std::string> displayName;

		LoginPacket() = default;

		LoginPacket(std::string_view username_, Token token_):
			username(username_), token(token_) {}

		LoginPacket(std::string_view username_, Token token_, std::string display_name):
			username(std::move(username_)), token(token_), displayName(std::move(display_name)) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << username << token << displayName; }
		void decode(Game &, Buffer &buffer)       override { buffer >> username >> token >> displayName; }

		void handle(const std::shared_ptr<ServerGame> &, RemoteClient &) override;
	};
}
