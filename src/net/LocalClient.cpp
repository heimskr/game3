#include "Log.h"
#include "game/ClientGame.h"
#include "net/Buffer.h"
#include "net/LocalClient.h"
#include "net/SocketBuffer.h"
#include "net/SSLSock.h"
#include "packet/Packet.h"

namespace Game3 {
	void LocalClient::connect(std::string_view hostname, uint16_t port) {
		stream.reset();
		buffer.reset();
		sock.reset();
		sock = std::make_shared<SSLSock>(hostname, port);
		sock->connect();
		buffer = std::make_shared<SocketBuffer>(sock);
		stream = std::make_shared<std::iostream>(buffer.get());
	}

	void LocalClient::read() {
		assert(stream);
		std::string input;

		if (!*stream)
			return;

		*stream >> input;

		if (input.empty())
			return;

		std::cout << '[' << input.size() << "]: \"" << input << "\"\n";
	}

	void LocalClient::send(std::string_view string) {
		*stream << string;
	}

	void LocalClient::send(const Packet &packet) {
		Buffer buffer;
		packet.encode(*game, buffer);
		assert(buffer.size() < UINT32_MAX);
		sendRaw(packet.getID());
		sendRaw(static_cast<uint32_t>(buffer.size()));
		*stream << buffer.str();
	}

	bool LocalClient::isConnected() const {
		return sock->isConnected();
	}
}
