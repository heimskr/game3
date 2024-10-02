#include "game/ClientGame.h"
#include "net/DirectLocalClient.h"
#include "net/DirectRemoteClient.h"

namespace Game3 {
	DirectLocalClient::DirectLocalClient() = default;

	void DirectLocalClient::send(const PacketPtr &packet) {
		std::shared_ptr<DirectRemoteClient> remote = getRemote();

		if (!remote) {
			throw std::runtime_error("Can't send packet to DirectRemoteClient: remote counterpart is missing");
		}

		remote->receive(packet);
	}

	void DirectLocalClient::receive(const PacketPtr &packet) {
		getGame()->queuePacket(packet);
	}
}
