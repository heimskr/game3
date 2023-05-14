#pragma once

#include "game/ChunkPosition.h"
#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class Player;

	class LoginStatusPacket: public Packet {
		public:
			static PacketID ID() { return 11; }

			bool success = false;
			std::string username;
			std::string displayName;

			LoginStatusPacket() = default;
			LoginStatusPacket(bool success_, std::string_view username_ = {}, std::string_view display_name = {}, std::shared_ptr<Player> = nullptr);

			PacketID getID() const override { return ID(); }

			void encode(Game &, Buffer &buffer) const override { buffer << success << username << displayName << playerDataBuffer; }
			void decode(Game &, Buffer &buffer)       override { buffer >> success >> username >> displayName >> playerDataBuffer; }

			void handle(ClientGame &) override;

		private:
			Buffer playerDataBuffer;
	};
}
