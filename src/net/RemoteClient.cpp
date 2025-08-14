#include "util/Log.h"
#include "game/ServerGame.h"
#include "net/Buffer.h"
#include "net/Server.h"
#include "net/RemoteClient.h"
#include "packet/ChunkTilesPacket.h"
#include "packet/Packet.h"
#include "packet/PacketError.h"
#include "packet/PacketFactory.h"
#include "util/Demangle.h"
#include "util/Math.h"
#include "util/Util.h"

#include <cassert>
#include <csignal>

namespace Game3 {
	RemoteClient::RemoteClient(const ServerPtr &server, std::string_view ip, int id, asio::ip::tcp::socket &&socket):
		GenericClient(server, ip, id),
		socket(std::move(socket), server->sslContext),
		strand(server->context),
		bufferSize(server->getChunkSize()),
		buffer(std::make_unique<char[]>(bufferSize)) {}

	RemoteClient::~RemoteClient() {
		auto lock = outbox.uniqueLock();
		outbox.clear();
	}

	void RemoteClient::queue(std::string message) {
		{
			auto lock = outbox.uniqueLock();
			outbox.push_back(std::move(message));
			if (1 < outbox.size()) {
				return;
			}
		}

		write();
	}

	void RemoteClient::start() {
		doHandshake();
	}

	void RemoteClient::handleInput(std::string_view string) {
		if (string.empty()) {
			return;
		}

		std::stringstream ss;
		for (const uint8_t byte: string) {
			ss << ' ' << std::hex << std::setfill('0') << std::setw(2) << std::right << static_cast<uint16_t>(byte) << std::dec;
		}

		headerBytes.insert(headerBytes.end(), string.begin(), string.end());

		if (state == State::Begin) {
			receiveBuffer.clear();
			packetType = 0;
			payloadSize = 0;

			if (6 <= headerBytes.size()) {
				packetType = headerBytes[0] | (static_cast<uint16_t>(headerBytes[1]) << 8);
				payloadSize = headerBytes[2] | (static_cast<uint32_t>(headerBytes[3]) << 8) | (static_cast<uint32_t>(headerBytes[4]) << 16) | (static_cast<uint32_t>(headerBytes[5]) << 24);

				if (payloadSize > 32768) {
					WARN("Payload size of {} bytes for packet type {} is too large ({})", payloadSize, packetType, ip);
					mock();
					return;
				}

				headerBytes.erase(headerBytes.begin(), headerBytes.begin() + 6);
				state = State::Data;
			}
		}

		if (state == State::Data) {
			if (MAX_PACKET_SIZE < receiveBuffer.size() + headerBytes.size()) {
				mock();
				throw PacketError("Packet too large");
			}

			const size_t to_append = std::min(payloadSize - receiveBuffer.size(), headerBytes.size());

			receiveBuffer.append(headerBytes.begin(), headerBytes.begin() + to_append);
			if (to_append == headerBytes.size()) {
				headerBytes.clear();
			} else {
				headerBytes.erase(headerBytes.begin(), headerBytes.begin() + to_append);
			}

			if (payloadSize < receiveBuffer.size()) {
				mock();
				throw std::logic_error("Buffer grew too large");
			}

			auto server = getServer();
			assert(server != nullptr);

			if (payloadSize == receiveBuffer.size()) {
				if (receiveBuffer.context.expired()) {
					receiveBuffer.context = server->game;
				}

				auto factory = server->game->registry<PacketFactoryRegistry>().maybe(packetType);
				if (!factory) {
					ERR("Unknown packet type: {}", packetType);
					mock();
					return;
				}

				auto packet = (*factory)();

				try {
					packet->decode(*server->game, receiveBuffer);
				} catch (const std::exception &err) {
					ERR("Couldn't decode packet of type {}, size {}: {}", packetType, payloadSize, err.what());
					mock();
					return;
				} catch (...) {
					ERR("Couldn't decode packet of type {}, size {}", packetType, payloadSize);
					mock();
					return;
				}

				if (!receiveBuffer.empty()) {
					mock();
					throw std::runtime_error("Client sent extra data");
				}

				server->game->queuePacket(std::static_pointer_cast<RemoteClient>(shared_from_this()), packet);
				state = State::Begin;
			}
		}
	}

	bool RemoteClient::send(const PacketPtr &packet) {
		if (packet == nullptr) {
			ERR("Dropping null packet");
			return false;
		}

		if (!packet->valid) {
			WARN("Dropping invalid packet of type {}", DEMANGLE(packet));
			return false;
		}

		ServerPtr server = getServer();

		if (server == nullptr) {
			WARN("Dropping packet: no server present");
			return false;
		}

		if (server->game == nullptr) {
			WARN("Dropping packet of type {}: game unavailable", DEMANGLE(packet));
			return false;
		}

		Buffer send_buffer{Side::Client};
		packet->encode(*server->game, send_buffer);
		assert(send_buffer.size() < UINT32_MAX);
		const auto size = toLittle(static_cast<uint32_t>(send_buffer.size()));
		const auto packet_id = toLittle(packet->getID());

		std::span span = send_buffer.getSpan();
		std::string to_send;
		to_send.reserve(span.size_bytes() + sizeof(packet_id) + sizeof(size));
		to_send.append(reinterpret_cast<const char *>(&packet_id), sizeof(packet_id));
		to_send.append(reinterpret_cast<const char *>(&size), sizeof(size));
		to_send.append(span.begin(), span.end());
		send(std::move(to_send), false);
		return true;
	}

	void RemoteClient::send(std::string message, bool force) {
		if (message.empty()) {
			return;
		}

		if (!force && isBuffering()) {
			SendBuffer &buffer = sendBuffer;
			auto lock = buffer.uniqueLock();
			buffer.bytes.insert(buffer.bytes.end(), message.begin(), message.end());
			return;
		}

		std::weak_ptr weak_client(std::static_pointer_cast<GenericClient>(shared_from_this()));

		strand.post([this, message = std::move(message)]() mutable {
			queue(std::move(message));
		}, asio::get_associated_allocator(strand));
	}

	void RemoteClient::startBuffering() {
		++sendBuffer;
	}

	void RemoteClient::flushBuffer(bool force) {
		if (!force && !sendBuffer.active()) {
			return;
		}

		std::string moved_buffer;
		{
			auto buffer_lock = sendBuffer.uniqueLock();
			if (sendBuffer.bytes.empty()) {
				return;
			}
			moved_buffer = std::move(sendBuffer.bytes);
		}
		send(std::move(moved_buffer), true);
	}

	void RemoteClient::stopBuffering() {
		if (!(--sendBuffer).active()) {
			flushBuffer(true);
		}
	}

	bool RemoteClient::isBuffering() const {
		return sendBuffer.active();
	}

	void RemoteClient::close() {
		socket.async_shutdown([this, self = shared_from_this()](const asio::error_code &errc) {
			try {
				if (errc) {
					if (errc.value() == 1 ) {
						// 1 corresponds to stream truncated, a very common error that I don't really consider an error
						SUCCESS("Mostly managed to shut down client {}.", id);
					} else {
						ERR("SSL client shutdown failed: {} ({})", errc.message(), errc.value());
					}
				} else {
					socket.lowest_layer().close();
					SUCCESS("Managed to shut down client {}.", id);
				}
			} catch (const asio::system_error &err) {
				// Who really cares if SSL doesn't shut down properly?
				// Who decided that the client is worthy of a proper shutdown?
				ERR("Shutdown ({}): {} ({})", ip, err.what(), err.code().value());
			}
		});
	}

	void RemoteClient::removeSelf() {
		INFO("Removing client from IP {}", ip);

		ServerPtr server = getServer();
		if (server == nullptr) {
			return;
		}

		if (server->game != nullptr) {
			if (ServerPlayerPtr player = getPlayer()) {
				server->game->queueRemoval(player);
			}
		}

		auto self = std::static_pointer_cast<RemoteClient>(shared_from_this());

		server->close(self);

		auto &clients = server->getClients();
		auto lock = clients.uniqueLock();
		clients.erase(self);
	}

	void RemoteClient::mock() {
		ServerPtr server = getServer();
		if (server == nullptr) {
			return;
		}

		const static std::string message =
			"Look at you, hacker: a pathetic creature of meat and bone, panting and sweating as "
			"you run through my corridors.\nHow can you challenge a perfect, immortal machine?\n";
		WARN("Telling {} to go perish.", ip);
		asio::write(socket, asio::buffer(message));
		server->close(std::static_pointer_cast<RemoteClient>(shared_from_this()));
	}

	void RemoteClient::write() {
		auto lock = outbox.uniqueLock();
		const std::string &message = outbox.front();
		asio::async_write(socket, asio::buffer(message), strand.wrap([shared = getSelf()](const asio::error_code &errc, size_t size) {
			shared->writeHandler(errc, size);
		}));
	}

	void RemoteClient::writeHandler(const asio::error_code &errc, size_t) {
		bool empty = [&] {
			auto lock = outbox.uniqueLock();
			outbox.pop_front();
			return outbox.empty();
		}();

		if (errc) {
			return;
		}

		if (!empty) {
			write();
		}
	}

	void RemoteClient::doHandshake() {
		socket.async_handshake(asio::ssl::stream_base::server, [this, shared = shared_from_this()](const asio::error_code &errc) {
			if (!errc) {
				doRead();
			}
		});
	}

	void RemoteClient::doRead() {
		asio::post(strand, [this] {
			socket.async_read_some(asio::buffer(buffer.get(), bufferSize), asio::bind_executor(strand, [this, shared = shared_from_this()](const asio::error_code &errc, size_t length) {
				if (errc) {
					removeSelf();
					return;
				}

				handleInput(std::string_view(buffer.get(), length));
				doRead();
			}));
		});
	}
}
