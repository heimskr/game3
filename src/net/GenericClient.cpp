#include "Log.h"
#include "net/GenericClient.h"
#include "net/Server.h"

namespace Game3 {
	GenericClient::GenericClient(Server &server_, std::string_view ip_, int id_, asio::ip::tcp::socket &&socket_):
		server(server_),
		id(id_),
		ip(ip_),
		socket(std::move(socket_), server.sslContext),
		strand(server.context),
		bufferSize(server_.getChunkSize()),
		buffer(std::make_unique<char[]>(bufferSize)) {}

	GenericClient::~GenericClient() {
		{
			auto lock = outbox.uniqueLock();
			outbox.clear();
		}

		INFOX(3, "\e[31m~GenericClient\e[39m({})", reinterpret_cast<void *>(this));
	}

	void GenericClient::start() {
		doHandshake();
	}

	void GenericClient::queue(std::string message) {
		{
			auto lock = outbox.uniqueLock();
			outbox.push_back(std::move(message));
			if (1 < outbox.size())
				return;
		}

		write();
	}

	void GenericClient::write() {
		auto lock = outbox.uniqueLock();
		const std::string &message = outbox.front();
		asio::async_write(socket, asio::buffer(message), strand.wrap([shared = shared_from_this()](const asio::error_code &errc, size_t size) {
			shared->writeHandler(errc, size);
		}));
	}

	void GenericClient::writeHandler(const asio::error_code &errc, size_t) {
		bool empty{};
		{
			auto lock = outbox.uniqueLock();
			outbox.pop_front();
			empty = outbox.empty();
		}

		if (errc) {
			ERROR("Client write ({}): {} ({})", ip, errc.message(), errc.value());
			return;
		}

		if (!empty)
			write();
	}

	void GenericClient::doHandshake() {
		socket.async_handshake(asio::ssl::stream_base::server, [this, shared = shared_from_this()](const asio::error_code &errc) {
			if (errc) {
				ERROR("Client handshake ({}): {}", ip, errc.message());
				return;
			}

			INFO("Handshake succeeded for {}", ip);
			doRead();
		});
	}

	void GenericClient::doRead() {
		asio::post(strand, [this] {
			socket.async_read_some(asio::buffer(buffer.get(), bufferSize), asio::bind_executor(strand, [this, shared = shared_from_this()](const asio::error_code &errc, size_t length) {
				if (errc) {
					if (errc.value() != 1) // "stream truncated"
						ERROR("Client read ({}): {} ({})", ip, errc.message(), errc.value());
					removeSelf();
					return;
				}

				handleInput(std::string_view(buffer.get(), length));
				doRead();
			}));
		});
	}
}
