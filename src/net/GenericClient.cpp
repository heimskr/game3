#include "Log.h"
#include "net/GenericClient.h"
#include "net/Server.h"
#include "realm/Realm.h"
#include "types/ChunkPosition.h"

namespace Game3 {
	GenericClient::GenericClient(Server &server):
		server(server) {}

	GenericClient::GenericClient(Server &server, std::string_view ip, int id):
		server(server),
		id(id),
		ip(ip) {}

	GenericClient::~GenericClient() {
		INFO(3, "\e[31m~GenericClient\e[39m({})", reinterpret_cast<void *>(this));
	}

	void GenericClient::sendChunk(Realm &realm, ChunkPosition chunk_position, bool can_request, uint64_t counter_threshold) {
		assert(server.game);

		if (counter_threshold != 0 && realm.tileProvider.contains(chunk_position) && realm.tileProvider.getUpdateCounter(chunk_position) < counter_threshold)
			return;

		try {
			realm.sendToOne(*this, chunk_position);
		} catch (const std::out_of_range &) {
			if (!can_request)
				throw;
			realm.requestChunk(chunk_position, shared_from_this());
		}
	}
}
