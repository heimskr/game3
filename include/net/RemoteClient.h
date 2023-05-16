#pragma once

#include <memory>

#include "net/Buffer.h"
#include "net/GenericClient.h"
#include "packet/Packet.h"

namespace Game3 {
	class LocalServer;
	class Player;

	/** Used by servers to represent a remote client. */
	class RemoteClient: public GenericClient, public std::enable_shared_from_this<RemoteClient> {
		public:
			constexpr static size_t MAX_PACKET_SIZE = 1 << 24;

			LocalServer &server;

			RemoteClient() = delete;
			RemoteClient(LocalServer &server_, int id_, std::string_view ip_):
				GenericClient(id_, ip_), server(server_) {}

			~RemoteClient() override = default;

			void handleInput(std::string_view) override;
			void send(const Packet &);
			void sendChunk(const Realm &, ChunkPosition);
			inline auto getPlayer() const { return weakPlayer.lock(); }
			inline void setPlayer(PlayerPtr shared) { weakPlayer = shared; }

			template <typename T>
			requires (!std::derived_from<T, Packet>)
			void send(const T &value);

		private:
			enum class State {Begin, Data};
			State state = State::Begin;
			std::vector<uint8_t> headerBytes;
			uint16_t packetType = 0;
			uint32_t payloadSize = 0;
			std::weak_ptr<Player> weakPlayer;
			Buffer buffer;
	};
}
