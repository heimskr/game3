#include "game/ClientGame.h"
#include "net/DirectLocalClient.h"
#include "net/DirectRemoteClient.h"

namespace Game3 {
	DirectLocalClient::DirectLocalClient() = default;

	void DirectLocalClient::send(const PacketPtr &packet) {
		std::shared_ptr<DirectRemoteClient> remote = getRemote();

		if (!remote) {
			// We're probably shutting down and the remote side has died before the local side.
			return;
		}

		sentPacketCounts.withUnique([&](std::map<PacketID, std::size_t> &counts) {
			++counts[packet->getID()];
		});

		remote->receive(packet);
	}

	void DirectLocalClient::connect(std::string_view, uint16_t) {
		// Connection requests go into a black hole. We're effectively already connected through shared memory.
	}

	void DirectLocalClient::disconnect() {
		// Same here.
	}

	bool DirectLocalClient::isConnected() const {
		return getRemote() != nullptr;
	}

	void DirectLocalClient::receive(const PacketPtr &packet) {
		receivedPacketCounts.withUnique([&](std::map<PacketID, std::size_t> &counts) {
			++counts[packet->getID()];
		});

		getGame()->queuePacket(packet);
	}
}
