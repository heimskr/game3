#include "Log.h"
#include "game/ClientGame.h"
#include "net/Buffer.h"
#include "net/LocalClient.h"
#include "packet/Packet.h"
#include "packet/PacketError.h"
#include "packet/PacketFactory.h"
#include "util/FS.h"
#include "util/Util.h"

#include <nlohmann/json.hpp>

#include <cassert>
#include <fstream>

namespace Game3 {
#ifdef USE_TLS
	namespace {
		asio::ssl::context makeSSLContext() {
			asio::ssl::context context(asio::ssl::context::tls_client);
			context.set_default_verify_paths();
			context.set_verify_mode(asio::ssl::verify_none);
			return context;
		}
	}

	LocalClient::LocalClient():
		ioContext(),
		sslContext(makeSSLContext()),
		workGuard(asio::make_work_guard(ioContext)),
		sslSock(ioContext, sslContext),
		strand(ioContext) {}
#else
	LocalClient::LocalClient() = default;
#endif

	LocalClient::~LocalClient() {
		INFO(3, "\e[31m~LocalClient\e[39m({})", reinterpret_cast<void *>(this));
		close();

		sslWaiter.wait();
		if (sslThread.joinable())
			sslThread.join();
	}

	void LocalClient::connect(std::string_view hostname, uint16_t port) {
		assert(!isReady());
#ifdef USE_TLS
		asio::io_service io_service;
		asio::ip::tcp::resolver resolver(io_service);

		auto resolved = resolver.resolve(hostname, "");
		if (resolved.size() != 1)
			throw std::runtime_error(std::format("Too {} resolution results: {}", resolved.empty()? "few" : "many", resolved.size()));
		lastHostname = hostname;
		auto endpoint = resolved->endpoint();
		endpoint.port(port);

		asio::post(strand, [this, endpoint] {
			sslSock.lowest_layer().async_connect(endpoint, asio::bind_executor(strand, [this, shared = shared_from_this()](const asio::error_code &errc) {
				if (reportError(errc))
					return;

				sslSock.async_handshake(asio::ssl::stream_base::client, [this](const asio::error_code &errc) {
					if (reportError(errc))
						return;
					sslReady = true;
					for (const std::function<void()> &action: connectionActions.steal())
						action();
					doRead();
				});
			}));
		});

		sslThread = std::thread([this] {
#if defined(__linux__) || defined(__APPLE__)
			pthread_setname_np(pthread_self(), "LocalClient ioContext");
#endif
			ioContext.run();
			--sslWaiter;
		});
#else
		INFO("Handshake done.");
		sock = std::make_shared<Sock>(hostname, port);
		sock->connect(true);
#endif
	}

	void LocalClient::read() {
		if (buffer.context.expired())
			if (ClientGamePtr game = getGame())
				buffer.context = game;

#ifndef USE_TLS
		assert(!reading.exchange(true));
		const auto byte_count = sock->recv(array.data(), array.size());
		handleInput(std::string_view(array.data(), byte_count));
#endif

		reading = false;
	}

	void LocalClient::send(const PacketPtr &packet) {
		Buffer send_buffer{Side::Server};
		auto game = getGame();
		send_buffer.context = game;
		packet->encode(*game, send_buffer);
		assert(send_buffer.size() < UINT32_MAX);
		const auto str = send_buffer.str();
		{
			std::unique_lock lock(packetMutex);
			sendRaw(packet->getID());
			sendRaw(static_cast<uint32_t>(send_buffer.size()));
			send(str.data(), str.size(), false);
		}
		bytesWritten += 6 + str.size();
		auto lock = sentPacketCounts.uniqueLock();
		++sentPacketCounts[packet->getID()];
	}

	bool LocalClient::isConnected() const {
#ifdef USE_TLS
		return sslReady;
#else
		return sock->isConnected();
#endif
	}

	std::shared_ptr<ClientGame> LocalClient::getGame() const {
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
#ifdef USE_TLS
		return isConnected();
#else
		return sock && !sock->hostname.empty();
#endif
	}

	const std::string & LocalClient::getHostname() const {
#ifdef USE_TLS
		if (!isConnected())
			throw std::runtime_error("Client not connected");
		return lastHostname;
#else
		if (!sock)
			throw std::runtime_error("Client not connected");
		return sock->hostname;
#endif
	}

	void LocalClient::setBuffering(bool new_value) {
#ifdef USE_TLS
		(void) new_value;
#else
		assert(sock);
		if (new_value)
			sock->startBuffering();
		else
			sock->stopBuffering();
#endif
	}

	bool LocalClient::isReady() const {
#ifdef USE_TLS
		return isConnected();
#else
		return sock->isReady();
#endif
	}

	void LocalClient::queueForConnect(std::function<void()> action) {
		if (isReady()) {
			action();
		} else {
			connectionActions.push(std::move(action));
		}
	}

#ifdef USE_TLS
	void LocalClient::write() {
		assert(isReady());
		std::string message;
		{
			auto lock = outbox.uniqueLock();
			message = std::move(outbox.front());
		}
		asio::async_write(sslSock, asio::buffer(message), strand.wrap([this, shared = shared_from_this()](const asio::error_code &errc, std::size_t) {
			bool empty{};
			{
				auto lock = outbox.uniqueLock();
				outbox.pop_front();
				empty = outbox.empty();
			}

			if (errc) {
				ERROR("LocalClient write ({}): {} ({})", lastHostname, errc.message(), errc.value());
			} else if (!empty) {
				write();
			}
		}));
	}

	bool LocalClient::reportError(const asio::error_code &errc) {
		if (errc) {
			if (onError)
				onError(errc);
			return true;
		}

		return false;
	}
#endif

	void LocalClient::send(const void *data, std::size_t size, bool force) {
#ifdef USE_TLS
		(void) force;
		{
			auto lock = outbox.uniqueLock();
			outbox.emplace_back(reinterpret_cast<const char *>(data), size);
			if (1 < outbox.size())
				return;
		}
		write();
#else
		sock->send(data, size, force);
#endif
	}

	void LocalClient::send(std::string message, bool force) {
#ifdef USE_TLS
		(void) force;
		{
			auto lock = outbox.uniqueLock();
			outbox.emplace_back(std::move(message));
			if (1 < outbox.size())
				return;
		}
		write();
#else
		sock->send(message.data(), message.size(), force);
#endif
	}

	void LocalClient::close() {
#ifdef USE_TLS
		if (isReady()) {
			sslSock.shutdown();
			sslReady = false;
		}

		workGuard.reset();
		ioContext.stop();
#else
		sock->close(false);
#endif
	}

#ifdef USE_TLS
	void LocalClient::doRead() {
		asio::post(strand, [this, shared = shared_from_this()] {
			if (!isReady()) {
				return;
			}

			reading = true;
			sslSock.async_read_some(asio::buffer(array), asio::bind_executor(strand, [this, shared](const asio::error_code &errc, std::size_t length) {
				if (errc && errc.value() != 2) {
					ERROR("LocalClient::doRead: {} ({})", errc.message(), errc.value());
					close();
					return;
				}

				handleInput(std::string_view(array.data(), length));
				doRead();
			}));
		});
	}
#endif

	void LocalClient::handleInput(std::string_view string) {
		if (weakGame.expired()) {
			return;
		}

		const std::size_t byte_count = string.size();

		if (byte_count <= 0) {
			reading = false;
			return;
		}

		bytesRead += static_cast<size_t>(byte_count);

		headerBytes.insert(headerBytes.end(), string.begin(), string.end());

		while (!headerBytes.empty()) {
			if (state == State::Begin) {
				buffer.clear();
				packetType = 0;
				payloadSize = 0;

				if (HEADER_SIZE <= headerBytes.size()) {
					packetType  = headerBytes[0] | (static_cast<uint16_t>(headerBytes[1]) << 8);
					payloadSize = headerBytes[2] | (static_cast<uint32_t>(headerBytes[3]) << 8) | (static_cast<uint32_t>(headerBytes[4]) << 16) | (static_cast<uint32_t>(headerBytes[5]) << 24);

					if (MAX_PACKET_SIZE <= payloadSize) {
						close();
						throw PacketError("Payload too large (" + std::to_string(payloadSize) + ')');
					}

					headerBytes.erase(headerBytes.begin(), headerBytes.begin() + HEADER_SIZE);
					state = State::Data;
				} else
					break;
			}

			if (state == State::Data) {
				if (headerBytes.empty())
					break;

				if (MAX_PACKET_SIZE < buffer.size() + headerBytes.size())
					throw PacketError("Packet too large");

				const size_t to_append = std::min(payloadSize - buffer.size(), headerBytes.size());

				buffer.append(headerBytes.begin(), headerBytes.begin() + to_append);
				headerBytes.erase(headerBytes.begin(), headerBytes.begin() + to_append);

				if (payloadSize < buffer.size())
					throw std::logic_error("Buffer grew too large");

				if (payloadSize == buffer.size()) {
					ClientGamePtr game = getGame();
					std::shared_ptr<Packet> packet = (*game->registry<PacketFactoryRegistry>().at(packetType))();
					packet->decode(*game, buffer);

					if (!buffer.empty()) {
						INFO("Bytes left in buffer: {} / {}", buffer.bytes.size() - buffer.skip, buffer.bytes.size());
						assert(buffer.empty());
					}

					buffer.clear();

					// In case a packet like TileEntityPacket moves the buffer.
					if (buffer.context.expired())
						buffer.context = game;

					state = State::Begin;
					{
						auto lock = receivedPacketCounts.uniqueLock();
						++receivedPacketCounts[packet->getID()];
					}
					game->queuePacket(std::move(packet));
				} else {
					break;
				}
			}
		}
	}

	void LocalClient::printHeaderBytes() const {
		INFO("HeaderBytes: ({})", hexString(headerBytes, true));
	}
}
