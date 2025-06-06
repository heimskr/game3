#include "util/Log.h"
#include "game/ServerGame.h"
#include "net/Buffer.h"
#include "net/RemoteClient.h"
#include "packet/ChunkRequestPacket.h"
#include "packet/PacketError.h"

namespace Game3 {
	ChunkRequestPacket::ChunkRequestPacket(Realm &realm, const std::set<ChunkPosition> &positions, bool no_threshold, bool generate_missing):
	realmID(realm.id),
	generateMissing(generate_missing) {
		for (const auto chunk_position: positions)
			requests.emplace(chunk_position, no_threshold? 0 : (realm.tileProvider.getUpdateCounter(chunk_position) + 1));
	}

	void ChunkRequestPacket::encode(Game &, Buffer &buffer) const {
		std::vector<uint32_t> data;
		data.reserve(4 * requests.size());

		for (const auto [chunk_position, counter]: requests) {
			data.push_back(chunk_position.x);
			data.push_back(chunk_position.y);
			data.push_back(counter & 0xffffffff);
			data.push_back((counter >> 32) & 0xffffffff);
		}

		buffer << realmID << generateMissing << data;
	}

	void ChunkRequestPacket::decode(Game &, Buffer &buffer) {
		std::vector<uint32_t> data;

		buffer >> realmID >> generateMissing >> data;

		if (data.empty())
			throw PacketError("Empty ChunkRequestPacket");

		if (data.size() % 4 != 0)
			throw PacketError("Misshapen ChunkRequestPacket");

		if (data.size() > 128)
			throw PacketError("Excessively greedy ChunkRequestPacket");

		requests.clear();
		for (size_t i = 0; i < data.size(); i += 4) {
			requests.emplace(
				ChunkPosition{static_cast<int32_t>(data[i]), static_cast<int32_t>(data[i + 1])},
				data[i + 2] | (static_cast<uint64_t>(data[i + 3]) << 32)
			);
		}
	}

	void ChunkRequestPacket::handle(const std::shared_ptr<ServerGame> &game, GenericClient &client) {
		if (generateMissing) {
			for (const ChunkRequest &request: requests)
				client.sendChunk(*game->getRealm(realmID), request.position, true, request.counterThreshold);
			return;
		}

		for (const ChunkRequest &request: requests) {
			try {
				client.sendChunk(*game->getRealm(realmID), request.position, false, request.counterThreshold);
			} catch (const std::out_of_range &) {}
		}
	}
}
