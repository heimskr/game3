#include <iostream>

#include "Log.h"
#include "entity/ClientPlayer.h"
#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "entity/ServerPlayer.h"
#include "error/InsufficientFundsError.h"
#include "game/ClientGame.h"
#include "game/ServerGame.h"
#include "game/Inventory.h"
#include "item/Tool.h"
#include "net/Buffer.h"
#include "net/LocalClient.h"
#include "net/RemoteClient.h"
#include "packet/AddKnownItemPacket.h"
#include "packet/RealmNoticePacket.h"
#include "packet/SetPlayerStationTypesPacket.h"
#include "realm/Realm.h"
#include "ui/Window.h"
#include "util/Cast.h"

namespace Game3 {
	Player::Player():
		Entity(ID()), LivingEntity() {}

	Player::~Player() {
		if (spawning)
			return;

		INFO(3, "\e[31m~Player\e[39m({}, {}, {})", reinterpret_cast<void *>(this), username.empty()? "[unknown username]" : username, globalID);
	}

	void Player::destroy() {
		std::weak_ptr weak(getShared());
		size_t times = 0;

		{
			auto lock = visibleEntities.sharedLock();
			if (!visibleEntities.empty()) {
				auto shared = getShared();
				for (const auto &weak_visible: visibleEntities)
					if (auto visible = weak_visible.lock())
						times += visible->removeVisible(weak);
			}
		}

		if (times != 0)
			INFO("Removed {} from visible sets {} time{}", username, times, times == 1? "" : "s");

		size_t remaining = 0;

		{
			auto ent_lock = getRealm()->entities.sharedLock();
			for (const auto &entity: getRealm()->entities) {
				auto vis_lock = entity->visiblePlayers.sharedLock();
				if (auto iter = entity->visiblePlayers.find(weak); iter != entity->visiblePlayers.end()) {
					++remaining;
					entity->visiblePlayers.erase(iter);
				}
			}
		}

		if (remaining != 0)
			ERROR("Player was still present in {} visible set{}!", remaining, remaining == 1? "" : "s");

		Entity::destroy();
	}

	HitPoints Player::getMaxHealth() const {
		return MAX_HEALTH;
	}

	void Player::toJSON(nlohmann::json &json) const {
		Entity::toJSON(json);
		LivingEntity::toJSON(json);
		json["isPlayer"] = true;
		json["displayName"] = displayName;
		json["spawnPosition"] = spawnPosition.copyBase();
		json["spawnRealmID"] = spawnRealmID;
		json["timeSinceAttack"] = timeSinceAttack;
		if (0.f < tooldown)
			json["tooldown"] = tooldown;
	}

	void Player::absorbJSON(const GamePtr &game, const nlohmann::json &json) {
		Entity::absorbJSON(game, json);
		LivingEntity::absorbJSON(game, json);
		displayName = json.at("displayName");
		spawnPosition = json.at("spawnPosition");
		spawnRealmID = json.at("spawnRealmID");
		timeSinceAttack = json.at("timeSinceAttack");
		if (auto iter = json.find("tooldown"); iter != json.end())
			tooldown = *iter;
		else
			tooldown = 0.f;
	}

	void Player::tick(const TickArgs &args) {
		Entity::tick(args);

		const auto delta = args.delta;

		if (0.f < tooldown) {
			tooldown -= delta;
			if (tooldown <= 0.f) {
				tooldown = 0;
				getInventory(0)->notifyOwner({});
			}
		}

		if (timeSinceAttack < 1'000'000.f)
			timeSinceAttack += delta;
	}

	bool Player::interactOn(Modifiers modifiers, const ItemStackPtr &used_item, Hand hand) {
		auto realm = getRealm();
		auto player = getShared();
		auto entity = realm->findEntity(position, player);
		if (!entity)
			return false;
		return entity->onInteractOn(player, modifiers, used_item, hand);
	}

	void Player::interactNextTo(Modifiers modifiers, const ItemStackPtr &used_item, Hand hand) {
		RealmPtr realm = getRealm();
		Position next_to = nextTo();
		PlayerPtr player = getShared();
		EntityPtr entity = realm->findEntity(next_to, player);
		bool interesting = false;

		if (hand != Hand::None && used_item && used_item->item->use(getHeldSlot(hand), used_item, getPlace(), modifiers, hand))
			return;

		if (entity)
			interesting = entity->onInteractNextTo(player, modifiers, used_item, hand);

		if (!interesting)
			if (auto tileEntity = realm->tileEntityAt(next_to))
				interesting = tileEntity->onInteractNextTo(player, modifiers, used_item, hand);

		if (!interesting)
			realm->interactGround(player, next_to, modifiers, used_item, hand);
	}

	void Player::teleport(const Position &position, const std::shared_ptr<Realm> &new_realm, MovementContext context) {
		GamePtr game = new_realm->getGame();

		if ((firstTeleport || weakRealm.lock() != new_realm) && getSide() == Side::Server) {
			clearOffset();
			stopMoving();
			notifyOfRealm(*new_realm);
		}

		RealmID old_realm_id = 0;
		auto locked_realm = weakRealm.lock();
		if (locked_realm)
			old_realm_id = locked_realm->id;

		Entity::teleport(position, new_realm, context);

		if ((old_realm_id == 0 || old_realm_id != nextRealm) && nextRealm != 0) {
			if (game->getSide() == Side::Client) {
				ClientGame &client_game = game->toClient();
				// Second condition is a hack. Sometimes the player gets interrealm teleported twice in the same tick.
				// TODO: figure out the reason for the above double interrealm teleportation.
				RealmPtr active_realm = client_game.getActiveRealm();
				if (getGID() == client_game.getPlayer()->getGID() && active_realm != new_realm) {
					active_realm->onBlur();
					active_realm->queuePlayerRemoval(getShared());
					active_realm = new_realm;
					active_realm->onFocus();
					client_game.setActiveRealm(std::move(active_realm));
					focus(*client_game.getWindow(), true);
					client_game.requestFromLimbo(new_realm->id);
				}
			} else {
				if (locked_realm)
					locked_realm->queuePlayerRemoval(getShared());
				auto locked_client = toServer()->weakClient.lock();
				assert(locked_client);
				if (!locked_client->getPlayer()->knowsRealm(new_realm->id)) {
					INFO("Sending {} to client for the first time", new_realm->id);
					new_realm->sendTo(*locked_client);
				}
			}
		}
	}

	bool Player::setTooldown(float multiplier) {
		if (getSide() != Side::Server)
			return false;

		if (ItemStackPtr active = getInventory(0)->getActive()) {
			if (auto tool = std::dynamic_pointer_cast<Tool>(active->item)) {
				tooldown = multiplier * tool->baseCooldown;
				increaseUpdateCounter();
				return true;
			}
		}

		return false;
	}

	float Player::getMovementSpeed() const {
		return isInFluid()? movementSpeed * .5f : movementSpeed;
	}

	void Player::give(const ItemStackPtr &stack, Slot start) {
		addKnownItem(stack);
		const InventoryPtr inventory = getInventory(0);
		auto lock = inventory->uniqueLock();
		if (ItemStackPtr leftover = inventory->add(stack, start)) {
			lock.unlock();
			getRealm()->spawn<ItemEntity>(getPosition(), leftover);
		}
	}

	Place Player::getPlace() {
		return {getPosition(), getRealm(), getShared()};
	}

	bool Player::isMoving() const {
		return movingUp || movingRight || movingDown || movingLeft;
	}

	bool Player::isMoving(Direction direction) const {
		switch (direction) {
			case Direction::Up:    return movingUp;
			case Direction::Right: return movingRight;
			case Direction::Down:  return movingDown;
			case Direction::Left:  return movingLeft;
			default: return false;
		}
	}

	void Player::setupRealm(const Game &game) {
		weakRealm = game.getRealm(realmID);
	}

	void Player::encode(Buffer &buffer) {
		Entity::encode(buffer);
		LivingEntity::encode(buffer);
		auto this_lock = sharedLock();
		buffer << displayName;
		buffer << tooldown;
		buffer << stationTypes;
		buffer << movementSpeed;
		buffer << spawnRealmID;
		buffer << spawnPosition;
		buffer << timeSinceAttack;
		if (buffer.target == Side::Server) {
			buffer << knownItems;
		}
	}

	void Player::decode(Buffer &buffer) {
		Entity::decode(buffer);
		LivingEntity::decode(buffer);
		auto this_lock = uniqueLock();
		buffer >> displayName;
		buffer >> tooldown;
		buffer >> stationTypes;
		buffer >> movementSpeed;
		buffer >> spawnRealmID;
		buffer >> spawnPosition;
		buffer >> timeSinceAttack;
		if (buffer.target == Side::Server) {
			buffer >> knownItems;
		}
		resetEphemeral();
	}

	void Player::startMoving(Direction direction) {
		switch (direction) {
			case Direction::Up:    movingUp    = true; break;
			case Direction::Right: movingRight = true; break;
			case Direction::Down:  movingDown  = true; break;
			case Direction::Left:  movingLeft  = true; break;
			default:
				return;
		}
	}

	void Player::stopMoving() {
		movingUp    = false;
		movingRight = false;
		movingDown  = false;
		movingLeft  = false;
	}

	void Player::stopMoving(Direction direction) {
		switch (direction) {
			case Direction::Up:    movingUp    = false; break;
			case Direction::Right: movingRight = false; break;
			case Direction::Down:  movingDown  = false; break;
			case Direction::Left:  movingLeft  = false; break;
			default:
				return;
		}
	}

	bool Player::send(const PacketPtr &packet) {
		if (getSide() == Side::Server) {
			if (auto locked = toServer()->weakClient.lock()) {
				locked->send(packet);
				return true;
			}
		} else {
			getGame()->toClient().getClient()->send(packet);
			return true;
		}

		return false;
	}

	void Player::addStationType(Identifier station_type) {
		{
			auto lock = stationTypes.uniqueLock();
			if (stationTypes.contains(station_type))
				return;
			stationTypes.insert(std::move(station_type));
		}
		send(make<SetPlayerStationTypesPacket>(stationTypes, true));
	}

	void Player::removeStationType(const Identifier &station_type) {
		{
			auto lock = stationTypes.uniqueLock();
			if (auto iter = stationTypes.find(station_type); iter != stationTypes.end())
				stationTypes.erase(station_type);
			else
				return;
		}
		send(make<SetPlayerStationTypesPacket>(stationTypes, false));
	}

	bool Player::hasStationType(const Identifier &station_type) const {
		auto lock = stationTypes.sharedLock();
		return stationTypes.contains(station_type);
	}

	PlayerPtr Player::getShared() {
		return safeDynamicCast<Player>(shared_from_this());
	}

	std::shared_ptr<ClientPlayer> Player::toClient() {
		return std::dynamic_pointer_cast<ClientPlayer>(shared_from_this());
	}

	std::shared_ptr<ServerPlayer> Player::toServer() {
		return std::dynamic_pointer_cast<ServerPlayer>(shared_from_this());
	}

	void Player::addKnownRealm(RealmID realm_id) {
		auto lock = knownRealms.uniqueLock();
		knownRealms.insert(realm_id);
	}

	bool Player::knowsRealm(RealmID realm_id) const {
		auto lock = knownRealms.sharedLock();
		return knownRealms.contains(realm_id);
	}

	void Player::notifyOfRealm(Realm &realm) {
		if (knowsRealm(realm.id))
			return;
		send(make<RealmNoticePacket>(realm));
		addKnownRealm(realm.id);
	}

	float Player::getAttackPeriod() const {
		if (speedStat == 0)
			return std::numeric_limits<float>::infinity();
		return 1 / speedStat;
	}

	bool Player::canAttack() const {
		return getAttackPeriod() <= timeSinceAttack;
	}

	bool Player::addKnownItem(const ItemStackPtr &stack) {
		return addKnownItem(stack->getID());
	}

	bool Player::addKnownItem(const Identifier &item_id) {
		bool inserted{};
		{
			auto lock = knownItems.uniqueLock();
			inserted = knownItems.emplace(item_id).second;
		}

		if (inserted && getSide() == Side::Server) {
			send(make<AddKnownItemPacket>(item_id));
		}

		return inserted;
	}

	void Player::setKnownItems(std::set<Identifier> item_ids) {
		knownItems = item_ids;
	}

	bool Player::hasKnownItem(const Identifier &item_id) const {
		auto lock = knownItems.sharedLock();
		return knownItems.contains(item_id);
	}

	void Player::resetEphemeral() {
		stopMoving();
		continuousInteraction = false;
		// `ticked` excluded intentionally. Probably.
		lastContinuousInteraction.reset();
		continuousInteractionModifiers = {};
	}

	void to_json(nlohmann::json &json, const Player &player) {
		player.toJSON(json);
	}
}
