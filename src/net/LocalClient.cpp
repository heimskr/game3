#include <cassert>
#include <fstream>

#include <nlohmann/json.hpp>

#include "Log.h"
#include "Options.h"
#include "game/ClientGame.h"
#include "net/Buffer.h"
#include "net/LocalClient.h"
#include "net/SocketBuffer.h"
#include "net/SSLSock.h"
#include "packet/Packet.h"
#include "packet/PacketError.h"
#include "packet/PacketFactory.h"
#include "util/FS.h"
#include "util/Util.h"

namespace Game3 {
	void LocalClient::connect(std::string_view hostname, uint16_t port) {
#ifdef USE_SSL
		sock = std::make_shared<SSLSock>(hostname, port);
#else
		sock = std::make_shared<Sock>(hostname, port);
#endif
		sock->connect(false);
	}

	void LocalClient::read() {
		if (buffer.context.expired())
			if (auto locked = lockGame())
				buffer.context = locked;

		std::array<char, 10240> array;
		const auto byte_count = sock->recv(array.data(), array.size());
		if (byte_count <= 0)
			return;

		std::string_view string(array.data(), byte_count);
		headerBytes.insert(headerBytes.end(), string.begin(), string.end());

		while (!headerBytes.empty()) {
			if (state == State::Begin) {
				buffer.clear();
				packetType = 0;
				payloadSize = 0;

				if (6 <= headerBytes.size()) {
					packetType = headerBytes[0] | (static_cast<uint16_t>(headerBytes[1]) << 8);
					payloadSize = headerBytes[2] | (static_cast<uint32_t>(headerBytes[3]) << 8) | (static_cast<uint32_t>(headerBytes[4]) << 16) | (static_cast<uint32_t>(headerBytes[5]) << 24);
					headerBytes.erase(headerBytes.begin(), headerBytes.begin() + 6);
					state = State::Data;
				} else
					break;
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
					auto game = lockGame();
					auto packet = (*game->registry<PacketFactoryRegistry>()[packetType])();
					packet->decode(*game, buffer);
					buffer.clear();
					game->queuePacket(std::move(packet));
					state = State::Begin;
				} else
					break;
			}
		}
	}

	void LocalClient::send(const Packet &packet) {
		Buffer send_buffer;
		auto game = lockGame();
		send_buffer.context = game;
		packet.encode(*game, send_buffer);
		assert(send_buffer.size() < UINT32_MAX);
		sendRaw(packet.getID());
		sendRaw(static_cast<uint32_t>(send_buffer.size()));
		const auto str = send_buffer.str();
		sock->send(str.c_str(), str.size());
	}

	bool LocalClient::isConnected() const {
		return sock->isConnected();
	}

	std::shared_ptr<ClientGame> LocalClient::lockGame() const {
		auto locked = weakGame.lock();
		assert(locked);
		return locked;
	}

	void LocalClient::setToken(const std::string &hostname, const std::string &username, Token token) {
		tokenDatabase[hostname][username] = token;
	}

	std::optional<Token> LocalClient::getToken(const std::string &hostname, const std::string &username) const {
		if (auto hostname_iter = tokenDatabase.find(hostname); hostname_iter != tokenDatabase.end()) {
			const auto &users = hostname_iter->second;
			if (auto username_iter = users.find(username); username_iter != users.end())
				return username_iter->second;
		}
		return std::nullopt;
	}

	void LocalClient::readTokens(const std::filesystem::path &path) {
		tokenDatabase = nlohmann::json::parse(readFile(path));
		tokenDatabasePath = path;
	}

	void LocalClient::saveTokens() const {
		assert(tokenDatabasePath);
		std::ofstream(*tokenDatabasePath) << nlohmann::json(tokenDatabase).dump();
	}

	void LocalClient::saveTokens(const std::filesystem::path &path) {
		tokenDatabasePath = path;
		std::ofstream(path) << nlohmann::json(tokenDatabase).dump();
	}

	bool LocalClient::hasHostname() const {
		return sock && !sock->hostname.empty();
	}

	const std::string & LocalClient::getHostname() const {
		if (!sock)
			throw std::runtime_error("Client not connected");
		return sock->hostname;
	}
}