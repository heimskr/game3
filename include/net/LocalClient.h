#pragma once

#include "Options.h"
#include "net/Buffer.h"
#include "packet/Packet.h"
#include "threading/Lockable.h"
#include "threading/MTQueue.h"
#include "threading/Waiter.h"
#include "types/Types.h"
#include "util/Math.h"

#include <asio.hpp>
#include <asio/ssl.hpp>

#include <atomic>
#include <deque>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <thread>

namespace Game3 {
	class ClientGame;

	class LocalClient: public std::enable_shared_from_this<LocalClient> {
		public:
			constexpr static size_t MAX_PACKET_SIZE = 1 << 20;
			constexpr static size_t HEADER_SIZE = 6;

			std::weak_ptr<ClientGame> weakGame;
			std::atomic_size_t bytesRead = 0;
			std::atomic_size_t bytesWritten = 0;
			std::function<void(const asio::error_code &)> onError;

			Lockable<std::map<PacketID, size_t>> receivedPacketCounts;
			Lockable<std::map<PacketID, size_t>> sentPacketCounts;

			LocalClient();

			virtual ~LocalClient();

			virtual void send(const PacketPtr &);

			virtual void connect(std::string_view hostname, uint16_t port);
			void read();
			virtual bool isConnected() const;
			std::shared_ptr<ClientGame> getGame() const;
			void setToken(const std::string &hostname, const std::string &username, Token);
			std::optional<Token> getToken(const std::string &hostname, const std::string &username) const;
			void readTokens(const std::filesystem::path &);
			void saveTokens() const;
			void saveTokens(std::filesystem::path);
			bool hasHostname() const;
			const std::string & getHostname() const;
			void setBuffering(bool);
			bool isReady() const;
			void queueForConnect(std::function<void()>);

		private:
			enum class State {Begin, Data};

			std::array<char, 16'384> array{};
			State state = State::Begin;
			Buffer buffer{Side::Client};
			uint16_t packetType = 0;
			uint32_t payloadSize = 0;
			std::deque<uint8_t> headerBytes;
			std::map<std::string, std::map<std::string, Token>> tokenDatabase;
			std::optional<std::filesystem::path> tokenDatabasePath;
			std::mutex packetMutex;
			std::atomic_bool reading = false;
			MTQueue<std::function<void()>> connectionActions;
#ifdef USE_TLS
			asio::io_context ioContext;
			asio::ssl::context sslContext;
			asio::executor_work_guard<asio::io_context::executor_type> workGuard;
			asio::ssl::stream<asio::ip::tcp::socket> sslSock;
			asio::io_context::strand strand;
			Lockable<std::deque<std::string>, std::shared_mutex> outbox;
			std::string lastHostname;
			bool sslReady = false;
			std::thread sslThread;
			Waiter sslWaiter{1};

			void doRead();
			void write();
			/** Returns whether the error was reported. */
			bool reportError(const asio::error_code &);
#else
#error "Non-SSL sockets are currently unsupported"
#endif

			void handleInput(std::string_view);
			void send(const void *, std::size_t, bool force);
			void send(std::string, bool force);
			void close();

			template <std::integral T>
			void sendRaw(T value) {
				T little = toLittle(value);
				bytesWritten += sizeof(T);
				send(&little, sizeof(little), false);
			}

			void printHeaderBytes() const;

		friend struct UsageCommand;
	};

	using LocalClientPtr = std::shared_ptr<LocalClient>;
}
