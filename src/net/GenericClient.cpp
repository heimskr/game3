#include "util/Log.h"
#include "net/GenericClient.h"
#include "net/Server.h"
#include "realm/Realm.h"
#include "types/ChunkPosition.h"

namespace Game3 {
	GenericClient::GenericClient(const ServerPtr &server):
		weakServer(server) {}

	GenericClient::GenericClient(const ServerPtr &server, std::string_view ip, int id):
		weakServer(server),
		id(id),
		ip(ip) {}

	GenericClient::~GenericClient() {
		INFO(3, "\e[31m~GenericClient\e[39m({})", reinterpret_cast<void *>(this));
	}

	void GenericClient::sendChunk(Realm &realm, ChunkPosition chunk_position, bool can_request, uint64_t counter_threshold) {
		auto server = getServer();
		assert(server != nullptr);
		assert(server->game != nullptr);

		if (counter_threshold != 0 && realm.tileProvider.contains(chunk_position) && realm.tileProvider.getUpdateCounter(chunk_position) < counter_threshold) {
			return;
		}

		if (realm.isChunkGenerated(chunk_position)) {
			try {
				realm.sendToOne(*this, chunk_position);
				return;
			} catch (const std::out_of_range &) {
				if (!can_request) {
					throw;
				}
			}
		}

		if (!can_request) {
			throw std::out_of_range("Cannot generate missing chunk");
		}

		realm.requestChunk(chunk_position, shared_from_this());
	}
}
