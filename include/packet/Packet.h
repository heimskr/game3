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

			Packet(const Packet &) = default;
			Packet(Packet &&) noexcept = default;

			virtual ~Packet() = default;

			Packet & operator=(const Packet &) = default;
			Packet & operator=(Packet &&) noexcept = default;

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
