#include "Log.h"
#include "net/Buffer.h"
#include "net/GameClient.h"
#include "net/GameServer.h"
#include "packet/Packet.h"
#include "util/Math.h"

namespace Game3 {
	void GameClient::handleInput(std::string_view string) {
		INFO("Handle[" << string << ']');
	}

	void GameClient::send(Packet &packet) {
		Buffer buffer;
		packet.encode(*server.game, buffer);
		send(packet.getID());
		send(buffer.size());
		send(buffer.str());
	}

	template <typename T>
	requires (!std::derived_from<T, Packet>)
	void GameClient::send(const T &value) {
		server.send(*this, value);
	}
}
