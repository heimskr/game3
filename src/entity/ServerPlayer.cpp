#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "entity/ServerPlayer.h"
#include "net/RemoteClient.h"
#include "packet/AgentMessagePacket.h"
#include "packet/EntityPacket.h"
#include "util/Cast.h"
#include "util/Util.h"

namespace Game3 {
	ServerPlayer::ServerPlayer():
		Entity(ID()), Player() {}

	ServerPlayer::~ServerPlayer() {
		Game &game = getGame();

		// If the game is being destroyed right now, we can't cast it.
		// The game is responsible for persisting all players before
		// the compiler-generated part of its destructor begins.
		if (game.dying)
			return;

		GameDB &database = game.toServer().database;
		if (database.isOpen()) {
			database.writeUser(*this);
			INFO("Persisted ServerPlayer with username " << username << '.');

			std::vector<std::string> usernames;

			{
				const auto &players = game.toServer().players;
				auto lock = players.sharedLock();
				if (players.empty()) {
					INFO("No remaining players.");
					return;
				}

				for (const ServerPlayerPtr &player: players)
					usernames.push_back(player->username);
			}

			INFO("Remaining player" << (usernames.size() == 1? "" : "s") << ": " << join(usernames, ", "));
		}
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

		RealmPtr realm = entity->getRealm();
		client->getPlayer()->notifyOfRealm(*realm);
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

	void ServerPlayer::kill() {
		WARN("Killing server player \e[1m" << username << "\e[22m.");
		ServerGame &game = getGame().toServer();

		const bool keep_inventory = game.getRule("keepInventory").value_or(1) != 0;

		if (!keep_inventory) {
			setHeldLeft(-1);
			setHeldRight(-1);
			InventoryPtr inventory = getInventory(0);
			auto lock = inventory->uniqueLock();
			inventory->iterate([&](ItemStack &stack, Slot) {
				stack.spawn(getRealm(), getPosition());
				return false;
			});
			inventory->clear();
			inventory->notifyOwner();
		}

		setHealth(getMaxHealth());

		// Can't do this immediately due to visibleChunks locking shenanigans.
		getRealm()->queue([&game, weak_player = std::weak_ptr(safeDynamicCast<ServerPlayer>(getSelf()))] {
			if (auto player = weak_player.lock()) {
				RealmPtr realm = game.getRealm(player->spawnRealmID);
				if (!realm) {
					WARN("Couldn't find spawn realm " << player->spawnRealmID << " for player " << player->username);
					return;
				}
				player->teleport(player->spawnPosition.copyBase(), realm, MovementContext{.facingDirection = Direction::Down, .isTeleport = true});
			}
		});
	}
}
