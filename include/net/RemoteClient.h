#pragma once

#include <memory>

#include "net/Buffer.h"
#include "net/GenericClient.h"
#include "packet/Packet.h"

namespace Game3 {
	class Realm;
	class ServerPlayer;
	struct ChunkPosition;

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

			RemoteClient() = delete;

			using GenericClient::GenericClient;

			~RemoteClient() override = default;

			void handleInput(std::string_view) override;
			bool send(const Packet &);
			void sendChunk(Realm &, ChunkPosition, bool can_request = true, uint64_t counter_threshold = 0);
			inline auto getPlayer() const { return weakPlayer.lock(); }
			inline void setPlayer(const std::shared_ptr<ServerPlayer> &shared) { weakPlayer = shared; }
			inline auto bufferGuard() { return BufferGuard(*this); }

			template <typename T>
			requires (!std::derived_from<T, Packet>)
			void send(const T &value);

			void startBuffering();
			void flushBuffer(bool force = false);
			void stopBuffering();
			bool isBuffering() const;

			void removeSelf() override;

		private:
			enum class State {Begin, Data};
			State state = State::Begin;
			std::vector<uint8_t> headerBytes;
			uint16_t packetType = 0;
			uint32_t payloadSize = 0;
			std::weak_ptr<ServerPlayer> weakPlayer;
			Buffer receiveBuffer;
	};
}
