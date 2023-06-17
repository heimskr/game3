#include <cassert>
#include <csignal>

#include "Log.h"
#include "game/ServerGame.h"
#include "net/Buffer.h"
#include "net/LocalServer.h"
#include "net/RemoteClient.h"
#include "packet/ChunkTilesPacket.h"
#include "packet/Packet.h"
#include "packet/PacketError.h"
#include "packet/PacketFactory.h"
#include "util/Math.h"
#include "util/Util.h"

namespace Game3 {
	void RemoteClient::handleInput(std::string_view string) {
		if (string.empty())
			return;

		std::stringstream ss;
		for (const uint8_t byte: string)
			ss << ' ' << std::hex << std::setfill('0') << std::setw(2) << std::right << static_cast<uint16_t>(byte) << std::dec;
		auto str = std::string(string);
		while (!str.empty() && (str.back() == '\r' || str.back() == '\n'))
			str.pop_back();

		headerBytes.insert(headerBytes.end(), string.begin(), string.end());

		if (state == State::Begin) {
			receiveBuffer.clear();
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
			if (MAX_PACKET_SIZE < receiveBuffer.size() + headerBytes.size())
				throw PacketError("Packet too large");

			const size_t to_append = std::min(payloadSize - receiveBuffer.size(), headerBytes.size());

			receiveBuffer.append(headerBytes.begin(), headerBytes.begin() + to_append);
			if (to_append == headerBytes.size())
				headerBytes.clear();
			else
				headerBytes.erase(headerBytes.begin(), headerBytes.begin() + to_append);

			if (payloadSize < receiveBuffer.size())
				throw std::logic_error("Buffer grew too large");

			if (payloadSize == receiveBuffer.size()) {
				if (receiveBuffer.context.expired())
					receiveBuffer.context = server.game;
				auto packet = (*server.game->registry<PacketFactoryRegistry>()[packetType])();

				try {
					packet->decode(*server.game, receiveBuffer);
				} catch (...) {
					ERROR("Couldn't decode packet of type " << packetType << ", size " << payloadSize);
					throw;
				}

				receiveBuffer.clear();
				server.game->queuePacket(shared_from_this(), packet);
				state = State::Begin;
			}
		}
	}

	bool RemoteClient::send(const Packet &packet) {
		if (!packet.valid) {
			WARN("Dropping invalid packet of type " << typeid(packet).name());
			return false;
		}

		assert(server.game);
		Buffer send_buffer;
		packet.encode(*server.game, send_buffer);
		assert(send_buffer.size() < UINT32_MAX);
		std::unique_lock lock(networkMutex);
		auto &bytes = send_buffer.getBytes();
		auto size = toLittle(static_cast<uint32_t>(send_buffer.size()));
		auto packet_id = toLittle(packet.getID());
		bytes.insert(bytes.begin(), reinterpret_cast<uint8_t *>(&size), reinterpret_cast<uint8_t *>(&size) + sizeof(size));
		bytes.insert(bytes.begin(), reinterpret_cast<uint8_t *>(&packet_id), reinterpret_cast<uint8_t *>(&packet_id) + sizeof(packet_id));
		send(send_buffer.str());
		return true;
	}

	void RemoteClient::sendChunk(Realm &realm, ChunkPosition chunk_position, bool can_request, uint64_t counter_threshold) {
		assert(server.game);

		if (counter_threshold != 0 && realm.tileProvider.contains(chunk_position) && realm.tileProvider.getUpdateCounter(chunk_position) < counter_threshold)
			return;

		try {
			send(ChunkTilesPacket(realm, chunk_position));
		} catch (const std::out_of_range &) {
			if (!can_request)
				throw;
			realm.requestChunk(chunk_position, shared_from_this());
		}
	}

	template <typename T>
	requires (!std::derived_from<T, Packet>)
	void RemoteClient::send(const T &value) {
		if (networkMutex.try_lock())
			throw std::runtime_error("Network mutex not locked...?");
		server.send(*this, value);
	}
}
