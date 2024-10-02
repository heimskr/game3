#pragma once

#include "net/LocalClient.h"

#include <memory>

namespace Game3 {
	class DirectRemoteClient;

	class DirectLocalClient: public LocalClient {
		public:
			DirectLocalClient();

			void send(const PacketPtr &) final;
			void receive(const PacketPtr &);

			inline auto getRemote() const { return weakRemote.lock(); }
			inline void setRemote(const std::shared_ptr<DirectRemoteClient> &shared) { weakRemote = shared; }

		private:
			std::weak_ptr<DirectRemoteClient> weakRemote;
	};
}
