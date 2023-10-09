#include "Log.h"
#include "net/GenericClient.h"
#include "net/Server.h"

namespace Game3 {
	GenericClient::GenericClient(Server &server_, std::string_view ip_, int id_, asio::ip::tcp::socket &&socket_):
		server(server_),
		id(id_),
		ip(ip_),
		socket(std::move(socket_), server.sslContext),
		bufferSize(server_.getChunkSize()),
		buffer(std::make_unique<char[]>(bufferSize)) {}

	void GenericClient::start() {
		doHandshake();
	}

	void GenericClient::doHandshake() {
		socket.async_handshake(asio::ssl::stream_base::server, [this](const asio::error_code &errc) {
			if (errc) {
				ERROR("Client handshake: " << errc.message());
				return;
			}

			INFO("Handshake succeeded for " << ip);
			doRead();
		});
	}

	void GenericClient::doRead() {
		socket.async_read_some(asio::buffer(buffer.get(), bufferSize), [this](const asio::error_code &errc, size_t length) {
			if (errc) {
				ERROR("Client read: " << errc.message());
				return;
			}

			handleInput(std::string_view(buffer.get(), length));
			doRead();
		});
	}
}
