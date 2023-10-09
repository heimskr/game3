#include "Log.h"
#include "net/GenericClient.h"
#include "net/Server.h"

namespace Game3 {
	GenericClient::GenericClient(Server &server_, std::string_view ip_, int id_, asio::ip::tcp::socket &&socket_):
		server(server_),
		id(id_),
		ip(ip_),
		socket(std::move(socket_)),
		sslStream(socket, server.sslContext) {}
}
