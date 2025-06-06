#include "game/ServerGame.h"
#include "net/DirectLocalClient.h"
#include "net/DirectRemoteClient.h"
#include "net/Server.h"

namespace Game3 {
	DirectRemoteClient::DirectRemoteClient(const ServerPtr &server):
		GenericClient(server) {}

	void DirectRemoteClient::start() {}

	bool DirectRemoteClient::send(const PacketPtr &packet) {
		std::shared_ptr<DirectLocalClient> local = getLocal();

		if (!local) {
			// We're probably shutting down and the local side has died before the remote side.
			return false;
		}

		local->receive(packet);
		return true;
	}

	void DirectRemoteClient::send(std::string, bool) {
		throw std::logic_error("void DirectRemoteClient::send(std::string, bool) must not be called");
	}

	void DirectRemoteClient::handleInput(std::string_view) {
		throw std::logic_error("void DirectRemoteClient::handleInput(std::string_view) must not be called");
	}

	bool DirectRemoteClient::isBuffering() const {
		return false;
	}

	void DirectRemoteClient::close() {}

	void DirectRemoteClient::removeSelf() {
		assert(!"TODO: implement DirectRemoteClient::removeSelf()");
	}

	std::unique_ptr<BufferGuard> DirectRemoteClient::bufferGuard() {
		return std::make_unique<BufferGuard>();
	}

	void DirectRemoteClient::receive(const PacketPtr &packet) {
		ServerPtr server = getServer();

		if (server == nullptr) {
			WARN("DirectRemoteClient can't receive packet: server has expired");
			return;
		}

		server->game->queuePacket(shared_from_this(), packet);
	}
}
