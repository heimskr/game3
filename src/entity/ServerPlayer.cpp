#include "game/ServerGame.h"
#include "entity/ServerPlayer.h"
#include "net/RemoteClient.h"
#include "packet/AgentMessagePacket.h"
#include "packet/EntityPacket.h"

namespace Game3 {
	ServerPlayer::ServerPlayer(): Player() {}

	ServerPlayer::~ServerPlayer() {
		nlohmann::json json;
		toJSON(json);
		getGame().toServer().database.writeUser(username, json);
		INFO("Persisted ServerPlayer with username " << username << '.');
	}

	std::shared_ptr<ServerPlayer> ServerPlayer::create(Game &) {
		return Entity::create<ServerPlayer>();
	}

	std::shared_ptr<ServerPlayer> ServerPlayer::fromJSON(Game &game, const nlohmann::json &json) {
		auto out = Entity::create<ServerPlayer>();
		out->absorbJSON(game, json);
		return out;
	}

	bool ServerPlayer::ensureEntity(const std::shared_ptr<Entity> &entity) {
		auto client = weakClient.lock();
		if (!client)
			return false;

		{
			auto lock = knownEntities.sharedLock();
			if (knownEntities.contains(std::weak_ptr(entity)))
				return false;
		}

		client->send(EntityPacket(entity));

		{
			auto lock = knownEntities.uniqueLock();
			knownEntities.insert(entity);
		}

		return true;
	}

	std::shared_ptr<RemoteClient> ServerPlayer::getClient() const {
		auto locked = weakClient.lock();
		assert(locked);
		return locked;
	}

	void ServerPlayer::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		assert(source);
		if (auto *buffer = std::any_cast<Buffer>(&data))
			send(AgentMessagePacket(source->getGID(), name, std::move(*buffer)));
		else
			throw std::runtime_error("Expected data to be a Buffer in ServerPlayer::handleMessage");
	}
}
