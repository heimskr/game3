#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "entity/ServerPlayer.h"
#include "net/RemoteClient.h"
#include "packet/AgentMessagePacket.h"
#include "packet/EntityMoneyChangedPacket.h"
#include "packet/EntityPacket.h"
#include "packet/EntitySetPathPacket.h"
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

		GamePtr game;

		try {
			game = getGame();
		} catch (const std::runtime_error &) {
			// We probably persisted past the death of the server.
			// This probably happens when a player uses the `stop` command.
			return;
		}

		// If the game is being destroyed right now, we can't cast it.
		// The game is responsible for persisting all players before
		// the compiler-generated part of its destructor begins.
		if (game->dying)
			return;

		GameDB &database = game->toServer().getDatabase();
		if (database.isOpen()) {
			database.writeUser(*this);
			INFO("Persisted {}.", username);

			std::vector<std::string> usernames;

			{
				const auto &players = game->toServer().players;
				auto lock = players.sharedLock();
				if (players.empty()) {
					INFO("No remaining players.");
					return;
				}

				for (const ServerPlayerPtr &player: players)
					usernames.push_back(player->username);
			}

			INFO("Remaining player{}: {}", usernames.size() == 1? "" : "s", join(usernames));
		}
	}

	std::shared_ptr<ServerPlayer> ServerPlayer::create(const std::shared_ptr<Game> &) {
		return Entity::create<ServerPlayer>();
	}

	std::shared_ptr<ServerPlayer> ServerPlayer::fromJSON(const GamePtr &game, const nlohmann::json &json) {
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

	void ServerPlayer::tick(const TickArgs &args) {
		Player::tick(args);

		if (continuousInteraction) {
			Place place = getPlace();
			if (!lastContinuousInteraction || *lastContinuousInteraction != place) {
				interactOn(Modifiers());
				getRealm()->interactGround(getShared(), position, continuousInteractionModifiers, nullptr, Hand::None);
				lastContinuousInteraction = std::move(place);
			}
		} else {
			lastContinuousInteraction.reset();
		}
	}

	void ServerPlayer::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		assert(source);
		if (auto *buffer = std::any_cast<Buffer>(&data))
			send(AgentMessagePacket(source->getGID(), name, std::move(*buffer)));
		else
			throw std::runtime_error("Expected data to be a Buffer in ServerPlayer::handleMessage");
	}

	void ServerPlayer::movedToNewChunk(const std::optional<ChunkPosition> &old_position) {
		auto shared = getShared();

		{
			auto lock = visibleEntities.sharedLock();
			for (const auto &weak_visible: visibleEntities) {
				if (auto visible = weak_visible.lock()) {
					if (!visible->path.empty() && visible->hasSeenPath(shared)) {
						// INFO("Late sending EntitySetPathPacket (Player)");
						toServer()->ensureEntity(visible);
						send(EntitySetPathPacket(*visible));
						visible->setSeenPath(shared);
					}

					if (!canSee(*visible)) {
						auto visible_lock = visible->visibleEntities.uniqueLock();
						visible->visiblePlayers.erase(shared);
						visible->visibleEntities.erase(shared);
					}
				}
			}
		}

		if (auto realm = weakRealm.lock()) {
			if (const auto client_ptr = toServer()->weakClient.lock()) {
				const auto chunk = getChunk();
				auto &client = *client_ptr;

				if (auto tile_entities = realm->getTileEntities(chunk)) {
					auto lock = tile_entities->sharedLock();
					for (const auto &tile_entity: *tile_entities)
						if (!tile_entity->hasBeenSentTo(shared))
							tile_entity->sendTo(client);
				}

				if (auto entities = realm->getEntities(chunk)) {
					auto lock = entities->sharedLock();
					for (const WeakEntityPtr &weak_entity: *entities)
						if (EntityPtr entity = weak_entity.lock(); entity && !entity->hasBeenSentTo(shared))
							entity->sendTo(client);
				}
			}

			Entity::movedToNewChunk(old_position);

			realm->recalculateVisibleChunks();
		} else {
			Entity::movedToNewChunk(old_position);
		}
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
		WARN("Killing server player \e[1m{}\e[22m.", username);
		ServerGame &game = getGame()->toServer();

		const bool keep_inventory = game.getRule("keepInventory").value_or(1) != 0;

		if (!keep_inventory) {
			setHeldLeft(-1);
			setHeldRight(-1);
			InventoryPtr inventory = getInventory(0);
			auto lock = inventory->uniqueLock();
			inventory->iterate([&](const ItemStackPtr &stack, Slot) {
				stack->spawn(getPlace());
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
					WARN("Couldn't find spawn realm {} for player {}", player->spawnRealmID, player->username);
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
