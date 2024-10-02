#include "game/ServerGame.h"
#include "net/DirectLocalClient.h"
#include "net/DirectRemoteClient.h"
#include "net/Server.h"

namespace Game3 {
	void DirectRemoteClient::handleInput(std::string_view) {
		throw std::logic_error("void DirectRemoteClient::handleInput(std::string_view) must not be called");
	}

	bool DirectRemoteClient::send(const PacketPtr &packet) {
		std::shared_ptr<DirectLocalClient> local = getLocal();

		if (!local) {
			throw std::runtime_error("Can't send packet to DirectRemoteClient: local counterpart is missing");
		}

		local->receive(packet);
		return true;
	}

	bool DirectRemoteClient::isBuffering() const {
		return false;
	}

	void DirectRemoteClient::receive(const PacketPtr &packet) {
		server.game->queuePacket(std::static_pointer_cast<RemoteClient>(shared_from_this()), packet);
	}
}
