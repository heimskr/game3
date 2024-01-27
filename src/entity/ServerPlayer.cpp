#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "entity/ServerPlayer.h"
#include "net/RemoteClient.h"
#include "packet/AgentMessagePacket.h"
#include "packet/EntityMoneyChangedPacket.h"
#include "packet/EntityPacket.h"
#include "util/Cast.h"
#include "util/Util.h"

namespace Game3 {
	ServerPlayer::ServerPlayer():
		Entity(ID()), Player() {}

	ServerPlayer::~ServerPlayer() {
		// In Tile::makeMonsterFactories, temporary entities are created so their virtual methods can be called.
		// Specifically, we want to check whether the entity is a type of monster. The entity is discarded
		// immediately after the check. It's marked as spawning before the check to indicate that it's in a
		// weird state. Therefore, we check here whether it's spawning to decide whether we should skip the
		// regular ServerPlayer destruction process.
		if (spawning)
			return;

		Game &game = getGame();

		// If the game is being destroyed right now, we can't cast it.
		// The game is responsible for persisting all players before
		// the compiler-generated part of its destructor begins.
		if (game.dying)
			return;

		GameDB &database = game.toServer().database;
		if (database.isOpen()) {
			database.writeUser(*this);
			INFO_("Persisted ServerPlayer with username " << username << '.');

			std::vector<std::string> usernames;

			{
				const auto &players = game.toServer().players;
				auto lock = players.sharedLock();
				if (players.empty()) {
					INFO_("No remaining players.");
					return;
				}

				for (const ServerPlayerPtr &player: players)
					usernames.push_back(player->username);
			}

			INFO_("Remaining player" << (usernames.size() == 1? "" : "s") << ": " << join(usernames, ", "));
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

	void ServerPlayer::addMoney(MoneyCount to_add) {
		setMoney(money + to_add);
	}

	bool ServerPlayer::removeMoney(MoneyCount to_remove) {
		if (money < to_remove)
			return false;

		setMoney(money - to_remove);
		return true;
	}

	void ServerPlayer::broadcastMoney() {
		Entity::broadcastMoney();
		send(EntityMoneyChangedPacket(*this));
	}

	void ServerPlayer::kill() {
		WARN_("Killing server player \e[1m" << username << "\e[22m.");
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
					WARN_("Couldn't find spawn realm " << player->spawnRealmID << " for player " << player->username);
					return;
				}
				player->teleport(player->spawnPosition.copyBase(), realm, MovementContext{.facingDirection = Direction::Down, .isTeleport = true});
			}
		});
	}

	void ServerPlayer::unsubscribeVillages() {
		if (auto village = subscribedVillage.lock())
			village->removeSubscriber(safeDynamicCast<ServerPlayer>(shared_from_this()));
		subscribedVillage.reset();
	}

	void ServerPlayer::subscribeVillage(const std::shared_ptr<Village> &village) {
		assert(village);
		village->addSubscriber(safeDynamicCast<ServerPlayer>(shared_from_this()));
		subscribedVillage = village;
	}
}
