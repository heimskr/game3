#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/AgentMessagePacket.h"
#include "packet/DisplayTextPacket.h"
#include "packet/EntityMoneyChangedPacket.h"
#include "packet/EntityPacket.h"
#include "packet/EntitySetPathPacket.h"
#include "util/Cast.h"
#include "util/Util.h"

namespace Game3 {
	ServerPlayer::ServerPlayer():
		Entity(ID()), Player() {
	}

	ServerPlayer::~ServerPlayer() {
		// In Tile::makeMonsterFactories, temporary entities are created so their virtual methods can be called.
		// Specifically, we want to check whether the entity is a type of monster. The entity is discarded
		// immediately after the check. It's marked as spawning before the check to indicate that it's in a
		// weird state. Therefore, we check here whether it's spawning to decide whether we should skip the
		// regular ServerPlayer destruction process.
		if (spawning) {
			return;
		}

		GamePtr game = weakGame.lock();

		if (!game) {
			if (RealmPtr realm = weakRealm.lock()) {
				game = realm->getGame();
			}
		}

		if (!game) {
			// We probably persisted past the death of the server.
			// This probably happens when a player uses the `stop` command.
			return;
		}

		// If the game is being destroyed right now, we can't cast it.
		// The game is responsible for persisting all players before
		// the compiler-generated part of its destructor begins.
		if (game->dying) {
			return;
		}

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

				for (const ServerPlayerPtr &player: players) {
					usernames.push_back(player->username);
				}
			}

			INFO("Remaining player{}: {}", usernames.size() == 1? "" : "s", join(usernames));
		}
	}

	std::shared_ptr<ServerPlayer> ServerPlayer::create(const std::shared_ptr<Game> &) {
		return Entity::create<ServerPlayer>();
	}

	std::shared_ptr<ServerPlayer> ServerPlayer::fromJSON(const GamePtr &game, const boost::json::value &json) {
		auto out = Entity::create<ServerPlayer>();
		out->absorbJSON(game, json);
		return out;
	}

	std::shared_ptr<ServerPlayer> ServerPlayer::fromBuffer(const GamePtr &game, Buffer &buffer) {
		auto out = Entity::create<ServerPlayer>();
		out->weakGame = game;
		out->decode(buffer);
		return out;
	}

	bool ServerPlayer::ensureEntity(const std::shared_ptr<Entity> &entity) {
		GenericClientPtr client = weakClient.lock();
		if (!client) {
			return false;
		}

		{
			auto lock = knownEntities.sharedLock();
			if (knownEntities.contains(std::weak_ptr(entity))) {
				return false;
			}
		}

		RealmPtr realm = entity->getRealm();
		client->getPlayer()->notifyOfRealm(*realm);
		client->send(make<EntityPacket>(entity));

		{
			auto lock = knownEntities.uniqueLock();
			knownEntities.insert(entity);
		}

		return true;
	}

	std::shared_ptr<GenericClient> ServerPlayer::getClient() const {
		GenericClientPtr client = weakClient.lock();
		assert(client);
		return client;
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
		if (Buffer *buffer = std::any_cast<Buffer>(&data)) {
			send(make<AgentMessagePacket>(source->getGID(), name, std::move(*buffer)));
		} else {
			throw std::runtime_error("Expected data to be a Buffer in ServerPlayer::handleMessage");
		}
	}

	void ServerPlayer::movedToNewChunk(const std::optional<ChunkPosition> &old_position) {
		PlayerPtr shared = getShared();

		{
			auto locks = getVisibleEntitiesLocks();
			if (visibleEntities) {
				for (const auto &weak_visible: *visibleEntities) {
					if (EntityPtr visible = weak_visible.lock()) {
						if (!visible->path.empty() && visible->hasSeenPath(shared)) {
							// INFO("Late sending EntitySetPathPacket (Player)");
							toServer()->ensureEntity(visible);
							send(make<EntitySetPathPacket>(*visible));
							visible->setSeenPath(shared);
						}

						if (!canSee(*visible)) {
							auto visible_lock = visible->visibleEntities.uniqueLock();
							visible->visiblePlayers.erase(shared);
							auto other_locks = visible->getVisibleEntitiesLocks();
							if (visible->visibleEntities) {
								visible->visibleEntities->erase(shared);
							}
						}
					}
				}
			}
		}

		if (auto realm = weakRealm.lock()) {
			if (const GenericClientPtr client_ptr = toServer()->weakClient.lock()) {
				const ChunkPosition chunk = getChunk();
				GenericClient &client = *client_ptr;

				if (auto tile_entities = realm->getTileEntities(chunk)) {
					auto lock = tile_entities->sharedLock();
					for (const auto &tile_entity: *tile_entities) {
						if (!tile_entity->hasBeenSentTo(shared)) {
							tile_entity->sendTo(client);
						}
					}
				}

				if (Realm::WeakEntitySet entities = realm->getEntities(chunk)) {
					auto lock = entities->sharedLock();
					for (const WeakEntityPtr &weak_entity: *entities) {
						if (EntityPtr entity = weak_entity.lock(); entity && !entity->hasBeenSentTo(shared)) {
							entity->sendTo(client);
						}
					}
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
		if (money < to_remove) {
			return false;
		}

		setMoney(money - to_remove);
		return true;
	}

	void ServerPlayer::broadcastMoney() {
		Entity::broadcastMoney();
		send(make<EntityMoneyChangedPacket>(*this));
	}

	void ServerPlayer::kill() {
		if (dying.exchange(true)) {
			WARN("Can't kill server player {}: already dying", username);
			return;
		}

		WARN("Killing server player \e[1m{}\e[22m.", username);
		ServerGame &game = getGame()->toServer();

		++kills;

		const bool keep_inventory = game.getRule("keepInventory").value_or(1) != 0;
		setStatusEffects({});

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
			inventory->notifyOwner({});
		}

		setHealth(getMaxHealth());

		// Can't do this immediately due to visibleChunks locking shenanigans.
		getRealm()->queue([&game, weak = getWeakSelf()] {
			if (ServerPlayerPtr player = weak.lock()) {
				RealmPtr realm = game.getRealm(player->spawnRealmID);
				if (!realm) {
					WARN("Couldn't find spawn realm {} for player {}", player->spawnRealmID, player->username);
					player->dying = false;
					return;
				}
				player->teleport(player->spawnPosition.copyBase(), realm, MovementContext{.facingDirection = Direction::Down, .isTeleport = true});
				player->onTeleported.connect([](const EntityPtr &entity) {
					safeDynamicCast<ServerPlayer>(entity)->dying = false;
				});
			}
		});
	}

	void ServerPlayer::setStatusEffects(StatusEffectMap effects) {
		if (dying && !effects.empty()) {
			return;
		}

		Player::setStatusEffects(std::move(effects));
	}

	bool ServerPlayer::susceptibleToStatusEffect(const Identifier &) const {
		return !dying;
	}

	void ServerPlayer::unsubscribeVillages() {
		if (VillagePtr village = subscribedVillage.lock()) {
			village->removeSubscriber(getSelf());
		}
		subscribedVillage.reset();
	}

	void ServerPlayer::subscribeVillage(const std::shared_ptr<Village> &village) {
		assert(village != nullptr);
		village->addSubscriber(getSelf());
		subscribedVillage = village;
	}

	void ServerPlayer::showText(const UString &text, const UString &name) {
		send(make<DisplayTextPacket>(name.raw(), text.raw(), true));
	}

	std::shared_ptr<ServerPlayer> ServerPlayer::getSelf() {
		return std::dynamic_pointer_cast<ServerPlayer>(shared_from_this());
	}

	std::weak_ptr<ServerPlayer> ServerPlayer::getWeakSelf() {
		return std::weak_ptr(getSelf());
	}
}
