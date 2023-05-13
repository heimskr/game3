#include "Log.h"
#include "game/ClientGame.h"
#include "net/Buffer.h"
#include "net/LocalClient.h"
#include "net/SocketBuffer.h"
#include "net/SSLSock.h"
#include "packet/Packet.h"
#include "packet/PacketError.h"
#include "packet/PacketFactory.h"

namespace Game3 {
	void LocalClient::connect(std::string_view hostname, uint16_t port) {
		sock = std::make_shared<SSLSock>(hostname, port);
		sock->connect(false);
	}

	void LocalClient::read() {
		std::array<char, 10240> array;
		const auto byte_count = sock->recv(array.data(), array.size());
		if (byte_count <= 0)
			return;

		std::string_view string(array.data(), byte_count);

		headerBytes.insert(headerBytes.end(), string.begin(), string.end());

		if (state == State::Begin) {
			buffer.clear();
			packetType = 0;
			payloadSize = 0;

			if (6 <= headerBytes.size()) {
				packetType = headerBytes[0] | (static_cast<uint16_t>(headerBytes[1]) << 8);
				payloadSize = headerBytes[2] | (static_cast<uint32_t>(headerBytes[3]) << 8) | (static_cast<uint32_t>(headerBytes[4]) << 16) | (static_cast<uint32_t>(headerBytes[5]) << 24);
				headerBytes.erase(headerBytes.begin(), headerBytes.begin() + 6);
				state = State::Data;
			}
		}

		if (state == State::Data) {
			if (MAX_PACKET_SIZE < buffer.size() + headerBytes.size())
				throw PacketError("Packet too large");

			const size_t to_append = std::min(payloadSize - buffer.size(), headerBytes.size());

			buffer.append(headerBytes.begin(), headerBytes.begin() + to_append);
			if (to_append == headerBytes.size())
				headerBytes.clear();
			else
				headerBytes.erase(headerBytes.begin(), headerBytes.begin() + to_append);

			if (payloadSize < buffer.size())
				throw std::logic_error("Buffer grew too large");

			if (payloadSize == buffer.size()) {
				assert(game);
				auto packet = (*game->registry<PacketFactoryRegistry>()[packetType])();
				packet->decode(*game, buffer);
				buffer.clear();
				game->queuePacket(std::move(packet));
				state = State::Begin;
			}
		}
	}

	void LocalClient::send(const Packet &packet) {
		Buffer buffer;
		packet.encode(*game, buffer);
		assert(buffer.size() < UINT32_MAX);
		sendRaw(packet.getID());
		sendRaw(static_cast<uint32_t>(buffer.size()));
		const auto str = buffer.str();
		sock->send(str.c_str(), str.size());
	}

	bool LocalClient::isConnected() const {
		return sock->isConnected();
	}
}
