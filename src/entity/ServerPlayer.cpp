#include "entity/ServerPlayer.h"
#include "net/RemoteClient.h"
#include "packet/EntityPacket.h"

namespace Game3 {
	ServerPlayer::ServerPlayer(): Player() {}

	std::shared_ptr<ServerPlayer> ServerPlayer::fromJSON(Game &game, const nlohmann::json &json) {
		auto out = Entity::create<ServerPlayer>();
		out->absorbJSON(game, json);
		out->init(game);
		return out;
	}

	bool ServerPlayer::ensureEntity(const std::shared_ptr<Entity> &entity) {
		{
			auto lock = knownEntities.sharedLock();
			if (knownEntities.contains(std::weak_ptr(entity)))
				return false;
		}

		getClient()->send(EntityPacket(entity));

		{
			auto lock = knownEntities.uniqueLock();
			knownEntities.insert(entity);
		}

		return true;
	}

	std::shared_ptr<RemoteClient> ServerPlayer::getClient() const {
		auto locked = client.lock();
		assert(locked);
		return locked;
	}
}
