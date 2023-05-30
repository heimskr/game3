#pragma once

#include <atomic>
#include <deque>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>

#include "Types.h"
#include "net/Buffer.h"
#include "net/Sock.h"
#include "util/Math.h"

namespace Game3 {
	class ClientGame;
	class Packet;

	class LocalClient: public std::enable_shared_from_this<LocalClient> {
		public:
			constexpr static size_t MAX_PACKET_SIZE = 1 << 24;

			std::weak_ptr<ClientGame> weakGame;
			size_t bytesRead = 0;
			size_t bytesWritten = 0;

			std::map<PacketID, size_t> receivedPacketCounts;
			std::shared_mutex receivedPacketCountsMutex;
			std::map<PacketID, size_t> sentPacketCounts;
			std::shared_mutex sentPacketCountsMutex;

			LocalClient() = default;

			void connect(std::string_view hostname, uint16_t port);
			void read();
			void send(const Packet &);
			bool isConnected() const;
			std::shared_ptr<ClientGame> lockGame() const;
			void setToken(const std::string &hostname, const std::string &username, Token);
			std::optional<Token> getToken(const std::string &hostname, const std::string &username) const;
			void readTokens(const std::filesystem::path &);
			void saveTokens() const;
			void saveTokens(const std::filesystem::path &);
			bool hasHostname() const;
			const std::string & getHostname() const;
			void setBuffering(bool);

		private:
			enum class State {Begin, Data};
			std::array<char, 1'000'000> array;
			State state = State::Begin;
			Buffer buffer;
			uint16_t packetType = 0;
			uint32_t payloadSize = 0;
			std::shared_ptr<Sock> sock;
			std::deque<uint8_t> headerBytes;
			std::map<std::string, std::map<std::string, Token>> tokenDatabase;
			std::optional<std::filesystem::path> tokenDatabasePath;
			std::mutex packetMutex;
			std::atomic_bool reading = false;

			template <std::integral T>
			void sendRaw(T value) {
				T little = toLittle(value);
				bytesWritten += sizeof(T);
				sock->send(&little, sizeof(little), false);
			}

			void printHeaderBytes() const;

			friend struct UsageCommand;
	};
}
