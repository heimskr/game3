#include <cassert>

#include "Log.h"
#include "game/ServerGame.h"
#include "net/Buffer.h"
#include "net/LocalServer.h"
#include "net/RemoteClient.h"
#include "packet/Packet.h"
#include "util/Math.h"

namespace Game3 {
	void RemoteClient::handleInput(std::string_view string) {
		INFO("Handle[" << string << ']');
	}

	void RemoteClient::send(const Packet &packet) {
		Buffer buffer;
		packet.encode(*server.game, buffer);
		assert(buffer.size() < UINT32_MAX);
		send(packet.getID());
		send(static_cast<uint32_t>(buffer.size()));
		send(buffer.str());
	}

	template <typename T>
	requires (!std::derived_from<T, Packet>)
	void RemoteClient::send(const T &value) {
		server.send(*this, value);
	}
}
