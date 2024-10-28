#pragma once

#include <format>
#include <memory>

#include "net/Buffer.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/Packet.h"

namespace Game3 {
	class DirectLocalClient;

	/** A direct client is one that is connected to the server through local play and exchanges packets directly in memory and not through a socket. */
	class DirectRemoteClient: public GenericClient {
		public:
			DirectRemoteClient(const std::shared_ptr<Server> &);

			void start() final;
			bool send(const PacketPtr &) final;
			void send(std::string, bool force) final;
			void handleInput(std::string_view) final;
			bool isBuffering() const final;
			void close() final;
			void removeSelf() final;
			std::unique_ptr<BufferGuard> bufferGuard() final;

			void receive(const PacketPtr &);

			inline auto getLocal() const { return weakLocal.lock(); }
			inline void setLocal(const std::shared_ptr<DirectLocalClient> &shared) { weakLocal = shared; }

		private:
			std::weak_ptr<DirectLocalClient> weakLocal;
	};
}
