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
	class DirectRemoteClient: public RemoteClient {
		public:
			DirectRemoteClient() = delete;

			using RemoteClient::RemoteClient;

			void handleInput(std::string_view) override;
			bool send(const PacketPtr &) final;
			bool isBuffering() const override;

			void receive(const PacketPtr &);

			inline auto getLocal() const { return weakLocal.lock(); }
			inline void setLocal(const std::shared_ptr<DirectLocalClient> &shared) { weakLocal = shared; }

		private:
			std::weak_ptr<DirectLocalClient> weakLocal;
	};
}
