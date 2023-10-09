#include "Log.h"
#include "net/GenericClient.h"
#include "net/Server.h"

namespace Game3 {
	GenericClient::GenericClient(Server &server_, std::string_view ip_, int id_):
		server(server_),
		id(id_),
		ip(ip_),
		socket(server_.context),
		sslStream(socket, server.sslContext) {}
}
