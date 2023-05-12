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
			std::string displayName;

			LoginStatusPacket() = default;
			LoginStatusPacket(bool success_, std::string_view display_name = {}, std::shared_ptr<Player> = nullptr);

			PacketID getID() const override { return ID(); }

			void encode(Game &, Buffer &buffer) const override { buffer << success << displayName << playerDataBuffer; }
			void decode(Game &, Buffer &buffer)       override { buffer >> success >> displayName >> playerDataBuffer; }

			void handle(ClientGame &) const override;

		private:
			Buffer playerDataBuffer;
	};
}
