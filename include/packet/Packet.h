#pragma once

#include "types/Types.h"
#include "util/Concepts.h"

namespace Game3 {
	class Buffer;
	class ClientGame;
	class Game;
	class GenericClient;
	class ServerGame;

	class Packet {
		public:
			Packet() = default;

			Packet(const Packet &) = delete;
			Packet(Packet &&) noexcept = delete;

			virtual ~Packet() = default;

			Packet & operator=(const Packet &) = delete;
			Packet & operator=(Packet &&) noexcept = delete;

			bool valid = true;

			virtual void encode(Game &, Buffer &) const = 0;
			virtual void decode(Game &, Buffer &) = 0;
			virtual PacketID getID() const = 0;

			virtual void handle(const std::shared_ptr<ServerGame> &, GenericClient &) {
				throw std::runtime_error("Packet " + std::to_string(getID()) + " cannot be handled server-side");
			}

			virtual void handle(const std::shared_ptr<ClientGame> &) {
				throw std::runtime_error("Packet " + std::to_string(getID()) + " cannot be handled client-side");
			}
	};

	using PacketPtr = std::shared_ptr<Packet>;

	template <typename P, typename... Args>
	requires std::derived_from<P, Packet>
	inline std::shared_ptr<P> make(Args &&...args) {
		return std::make_shared<P>(std::forward<Args>(args)...);
	}

	template <typename T>
	concept IsPacketPtr = IsDerivedPtr<T, Packet>;
}
