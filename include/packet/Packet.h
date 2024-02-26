#pragma once

#include "types/Types.h"

namespace Game3 {
	class Buffer;
	class ClientGame;
	class Game;
	class RemoteClient;
	class ServerGame;

	class Packet {
		public:
			Packet() = default;
			virtual ~Packet() = default;

			bool valid = true;

			virtual void encode(Game &, Buffer &) const = 0;
			virtual void decode(Game &, Buffer &) = 0;
			virtual PacketID getID() const = 0;

			virtual void handle(const std::shared_ptr<ServerGame> &, RemoteClient &) {
				throw std::runtime_error("Packet " + std::to_string(getID()) + " cannot be handled server-side");
			}

			virtual void handle(const std::shared_ptr<ClientGame> &) {
				throw std::runtime_error("Packet " + std::to_string(getID()) + " cannot be handled client-side");
			}
	};
}
