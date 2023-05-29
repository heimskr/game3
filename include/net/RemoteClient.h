#pragma once

#include <memory>

#include "net/Buffer.h"
#include "net/GenericClient.h"
#include "net/LocalServer.h"
#include "packet/Packet.h"

namespace Game3 {
	class Player;
	class Realm;
	struct ChunkPosition;

	/** Used by servers to represent a remote client. */
	class RemoteClient: public GenericClient, public std::enable_shared_from_this<RemoteClient> {
		public:
			struct BufferGuard {
				std::weak_ptr<RemoteClient> parent;

				BufferGuard(RemoteClient &parent_):
					BufferGuard(parent_.shared_from_this()) {}

				BufferGuard(const std::shared_ptr<RemoteClient> &parent_): parent(parent_) {
					parent_->startBuffering();
				}

				~BufferGuard() {
					if (auto locked = parent.lock())
						locked->stopBuffering();
				}
			};

			constexpr static size_t MAX_PACKET_SIZE = 1 << 24;

			LocalServer &server;

			RemoteClient() = delete;
			RemoteClient(LocalServer &server_, int id_, int fd, std::string_view ip_, bufferevent *event):
				GenericClient(*server_.server, id_, fd, ip_, event), server(server_) {}

			~RemoteClient() override = default;

			void handleInput(std::string_view) override;
			void send(const Packet &);
			void sendChunk(Realm &, ChunkPosition, bool can_request = true);
			inline auto getPlayer() const { return weakPlayer.lock(); }
			inline void setPlayer(PlayerPtr shared) { weakPlayer = shared; }
			inline auto bufferGuard() { return BufferGuard(*this); }

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
			std::mutex packetMutex;
	};
}
