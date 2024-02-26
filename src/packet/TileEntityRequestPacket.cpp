#include "entity/Entity.h"
#include "game/ServerGame.h"
#include "net/Buffer.h"
#include "net/RemoteClient.h"
#include "packet/DestroyTileEntityPacket.h"
#include "packet/ErrorPacket.h"
#include "packet/PacketError.h"
#include "packet/TileEntityRequestPacket.h"

namespace Game3 {
	TileEntityRequest::TileEntityRequest(TileEntity &tile_entity):
		TileEntityRequest(tile_entity.getGID(), tile_entity.getUpdateCounter() + 1) {}

	TileEntityRequestPacket::TileEntityRequestPacket(Realm &realm, const std::set<ChunkPosition> &positions):
	realmID(realm.id) {
		for (const auto chunk_position: positions) {
			if (auto tile_entities = realm.getTileEntities(chunk_position)) {
				auto lock = tile_entities->sharedLock();
				for (const auto &tile_entity: *tile_entities)
					requests.emplace_back(tile_entity->getGID(), tile_entity->getUpdateCounter() + 1);
			}
		}
	}

	void TileEntityRequestPacket::encode(Game &, Buffer &buffer) const {
		std::vector<uint64_t> data;
		data.reserve(2 * requests.size());

		for (const auto [tile_entity_id, threshold]: requests) {
			data.push_back(tile_entity_id);
			data.push_back(threshold);
		}

		buffer << realmID << data;
	}

	void TileEntityRequestPacket::decode(Game &, Buffer &buffer) {
		std::vector<uint64_t> data;

		buffer >> realmID >> data;

		if (data.empty())
			throw PacketError("Empty TileEntityRequestPacket");

		if (data.size() % 2 != 0)
			throw PacketError("Misshapen TileEntityRequestPacket");

		if (data.size() > 4096 * 2)
			throw PacketError("Excessively greedy TileEntityRequestPacket");

		requests.clear();
		for (size_t i = 0; i < data.size(); i += 2)
			requests.emplace_back(data[i], data[i + 1]);
	}

	void TileEntityRequestPacket::handle(const std::shared_ptr<ServerGame> &game, RemoteClient &client) {
		RealmPtr realm = game.tryRealm(realmID);
		if (!realm) {
			client.send(ErrorPacket("Invalid realm"));
			return;
		}

		auto lock = realm->tileEntitiesByGID.sharedLock();

		for (const auto [tile_entity_id, threshold]: requests) {
			// If the tile entity is found in this realm, try to send it if its update counter is higher than the threshold.
			// Otherwise, try to get the client to destroy it. If the tile entity does exist but is in another realm,
			// everything will be taken care of once the player enters that realm.
			if (auto iter = realm->tileEntitiesByGID.find(tile_entity_id); iter != realm->tileEntitiesByGID.end())
				iter->second->sendTo(client, threshold);
			else
				client.send(DestroyTileEntityPacket(tile_entity_id));
		}
	}
}
