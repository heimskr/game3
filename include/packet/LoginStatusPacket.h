#pragma once

#include "types/ChunkPosition.h"
#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class Player;

	class LoginStatusPacket: public Packet {
		public:
			static PacketID ID() { return 11; }

			bool success = false;
			GlobalID globalID = -1;
			std::string username;
			std::string displayName;
			std::string message;

			LoginStatusPacket() = default;
			LoginStatusPacket(bool success, GlobalID, std::string username, std::string display_name, std::shared_ptr<Player> = nullptr);
			/** For login failure */
			explicit LoginStatusPacket(std::string message);

			PacketID getID() const override { return ID(); }

			void encode(Game &, Buffer &) const override;
			void decode(Game &, Buffer &)       override;

			void handle(const std::shared_ptr<ClientGame> &) override;

		private:
			Buffer playerDataBuffer{Side::Client};
	};
}
