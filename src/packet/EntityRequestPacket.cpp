#include "entity/Entity.h"
#include "game/ServerGame.h"
#include "net/Buffer.h"
#include "net/RemoteClient.h"
#include "packet/DestroyEntityPacket.h"
#include "packet/EntityRequestPacket.h"
#include "packet/ErrorPacket.h"
#include "packet/PacketError.h"

namespace Game3 {
	EntityRequest::EntityRequest(Entity &entity):
		entityID(entity.getGID()), threshold(entity.getUpdateCounter() + 1) {}

	EntityRequestPacket::EntityRequestPacket(Realm &realm, const std::set<ChunkPosition> &positions):
	realmID(realm.id) {
		for (const auto chunk_position: positions) {
			if (auto entities = realm.getEntities(chunk_position)) {
				auto lock = entities->sharedLock();
				for (const auto &entity: *entities)
					requests.emplace_back(entity->getGID(), entity->getUpdateCounter() + 1);
			}
		}
	}

	void EntityRequestPacket::encode(Game &, Buffer &buffer) const {
		std::vector<uint64_t> data;
		data.reserve(2 * requests.size());

		for (const auto [entity_id, threshold]: requests) {
			data.push_back(entity_id);
			data.push_back(threshold);
		}

		buffer << realmID << data;
	}

	void EntityRequestPacket::decode(Game &, Buffer &buffer) {
		std::vector<uint64_t> data;

		buffer >> realmID >> data;

		if (data.empty())
			throw PacketError("Empty EntityRequestPacket");

		if (data.size() % 2 != 0)
			throw PacketError("Misshapen EntityRequestPacket");

		if (data.size() > 4096 * 2)
			throw PacketError("Excessively greedy EntityRequestPacket");

		requests.clear();
		for (size_t i = 0; i < data.size(); i += 2)
			requests.emplace_back(data[i], data[i + 1]);
	}

	void EntityRequestPacket::handle(ServerGame &game, RemoteClient &client) {
		auto realm_iter = game.realms.find(realmID);
		if (realm_iter == game.realms.end()) {
			client.send(ErrorPacket("Invalid realm"));
			return;
		}

		auto realm = realm_iter->second;
		auto lock = realm->entitiesByGID.sharedLock();

		for (const auto [entity_id, threshold]: requests) {
			// If the entity is found in this realm, try to send it if its update counter is higher than the threshold.
			// Otherwise, try to get the client to destroy it. If the entity does exist but is in another realm,
			// everything will be taken care of once the player enters that realm.
			if (auto iter = realm->entitiesByGID.find(entity_id); iter != realm->entitiesByGID.end())
				iter->second->sendTo(client, threshold);
			else
				client.send(DestroyEntityPacket(entity_id));
		}
	}
}
