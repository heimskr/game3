#include <iostream>

#include "Log.h"
#include "entity/ClientPlayer.h"
#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "entity/ServerPlayer.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "item/Tool.h"
#include "net/Buffer.h"
#include "net/LocalClient.h"
#include "net/RemoteClient.h"
#include "packet/ChunkRequestPacket.h"
#include "packet/EntityRequestPacket.h"
#include "packet/EntitySetPathPacket.h"
#include "packet/RealmNoticePacket.h"
#include "packet/SetPlayerStationTypesPacket.h"
#include "packet/TileEntityRequestPacket.h"
#include "realm/Realm.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/tab/TextTab.h"

namespace Game3 {
	Player::Player():
		Entity(ID()) {}

	Player::~Player() {
		INFO("~Player(" << this << ", " << globalID << ')');
	}

	void Player::destroy() {
		auto lock = visibleEntities.sharedLock();

		size_t times = 0;

		if (!visibleEntities.empty()) {
			auto shared = getShared();
			for (const auto &weak_visible: visibleEntities)
				if (auto visible = weak_visible.lock())
					times += visible->removeVisible(std::weak_ptr(shared));
		}

		INFO("Removed from visible sets " << times << " time(s)");

		size_t remaining = 0;
		{
			auto ent_lock = getRealm()->entities.sharedLock();
			for (const auto &entity: getRealm()->entities) {
				auto vis_lock = entity->visibleEntities.sharedLock();
				remaining += entity->visiblePlayers.contains(std::weak_ptr(getShared()));
			}
		}

		if (remaining == 0)
			SUCCESS("No longer present in any visible sets.");
		else
			ERROR("Still present in " << remaining << " visible set" << (remaining == 1? "" : "s") << '!');

		Entity::destroy();
	}

	void Player::toJSON(nlohmann::json &json) const {
		Entity::toJSON(json);
		json["isPlayer"] = true;
		json["displayName"] = displayName;
		if (0.f < tooldown)
			json["tooldown"] = tooldown;
	}

	void Player::absorbJSON(Game &game, const nlohmann::json &json) {
		Entity::absorbJSON(game, json);
		displayName = json.at("displayName");
		if (auto iter = json.find("tooldown"); iter != json.end())
			tooldown = *iter;
		else
			tooldown = 0.f;
	}

	void Player::tick(Game &game, float delta) {
		Entity::tick(game, delta);

		if (0.f < tooldown) {
			tooldown -= delta;
			if (tooldown < 0.f) {
				tooldown = 0;
				getInventory()->notifyOwner();
			}
		}

		if (getSide() == Side::Client) {
			Direction final_direction = direction;

			if (movingLeft && !movingRight)
				final_direction = Direction::Left;

			if (movingRight && !movingLeft)
				final_direction = Direction::Right;

			if (movingUp && !movingDown)
				final_direction = Direction::Up;

			if (movingDown && !movingUp)
				final_direction = Direction::Down;

			const MovementContext context {
				.clearOffset = false,
				.facingDirection = final_direction
			};

			if (movingLeft && !movingRight)
				move(Direction::Left, context);

			if (movingRight && !movingLeft)
				move(Direction::Right, context);

			if (movingUp && !movingDown)
				move(Direction::Up, context);

			if (movingDown && !movingUp)
				move(Direction::Down, context);

			direction = final_direction;
		} else {
			if (continuousInteraction) {
				Place place = getPlace();
				if (!lastContinuousInteraction || *lastContinuousInteraction != place) {
					interactOn(Modifiers());
					getRealm()->interactGround(getShared(), position, continuousInteractionModifiers);
					lastContinuousInteraction = std::move(place);
				}
			} else {
				lastContinuousInteraction.reset();
			}
		}
	}

	bool Player::interactOn(Modifiers modifiers) {
		auto realm = getRealm();
		auto player = getShared();
		auto entity = realm->findEntity(position, player);
		if (!entity)
			return false;
		return entity->onInteractOn(player, modifiers);
	}

	void Player::interactNextTo(Modifiers modifiers) {
		auto realm = getRealm();
		const Position next_to = nextTo();
		auto player = getShared();
		auto entity = realm->findEntity(next_to, player);
		bool interesting = false;
		if (entity)
			interesting = entity->onInteractNextTo(player, modifiers);
		if (!interesting)
			if (auto tileEntity = realm->tileEntityAt(next_to))
				interesting = tileEntity->onInteractNextTo(player, modifiers);
		if (!interesting)
			realm->interactGround(player, next_to, modifiers);
	}

	void Player::teleport(const Position &position, const std::shared_ptr<Realm> &new_realm, MovementContext context) {
		auto &game = new_realm->getGame();

		if ((firstTeleport || weakRealm.lock() != new_realm) && getSide() == Side::Server) {
			clearOffset();
			stopMoving();
			notifyOfRealm(*new_realm);
		}

		RealmID old_realm_id = -1;
		auto locked_realm = weakRealm.lock();
		if (locked_realm)
			old_realm_id = locked_realm->id;

		Entity::teleport(position, new_realm, context);

		if ((old_realm_id == -1 || old_realm_id != nextRealm) && nextRealm != -1) {
			if (getSide() == Side::Client) {
				auto &client_game = game.toClient();
				if (getGID() == client_game.player->getGID()) {
					{
						client_game.activeRealm->onBlur();
						client_game.activeRealm->queuePlayerRemoval(getShared());
						client_game.activeRealm = new_realm;
						client_game.activeRealm->onFocus();
					}
					focus(game.toClient().canvas, true);
					client_game.requestFromLimbo(new_realm->id);
				}
			} else {
				if (locked_realm)
					locked_realm->queuePlayerRemoval(getShared());
				auto locked_client = toServer()->weakClient.lock();
				assert(locked_client);
				INFO("Sending " << new_realm->id << " to client");
				new_realm->sendTo(*locked_client);
			}
		}
	}

	void Player::addMoney(MoneyCount to_add) {
		money += to_add;
		auto &game = getRealm()->getGame();
		if (game.getSide() == Side::Client)
			game.toClient().signal_player_money_update().emit(getShared());
		else
			increaseUpdateCounter();
	}

	bool Player::setTooldown(float multiplier) {
		if (getSide() != Side::Server)
			return false;

		if (auto *active = getInventory()->getActive())
			if (auto tool = std::dynamic_pointer_cast<Tool>(active->item)) {
				tooldown = multiplier * tool->baseCooldown;
				increaseUpdateCounter();
				return true;
			}
		return false;
	}

	void Player::showText(const Glib::ustring &text, const Glib::ustring &name) {
		if (getSide() == Side::Client) {
			getRealm()->getGame().toClient().setText(text, name, true, true);
			queueForMove([player = shared_from_this()](const auto &) {
				// TODO!: nanogui
				// player->getRealm()->getGame().toClient().canvas.window.textTab->hide();
				return true;
			});
		}
	}

	void Player::give(const ItemStack &stack, Slot start) {
		const InventoryPtr inventory = getInventory();
		auto lock = inventory->uniqueLock();
		if (auto leftover = inventory->add(stack, start)) {
			lock.unlock();
			getRealm()->spawn<ItemEntity>(getPosition(), *leftover);
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
		buffer << displayName;
		buffer << tooldown;
		buffer << stationTypes;
		buffer << speed;
	}

	void Player::decode(Buffer &buffer) {
		Entity::decode(buffer);
		buffer >> displayName;
		buffer >> tooldown;
		buffer >> stationTypes;
		buffer >> speed;
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

		// if (getSide() == Side::Client)
		// 	getGame().toClient().client->send(StartPlayerMovementPacket(direction));
	}

	void Player::stopMoving() {
		movingUp    = false;
		movingRight = false;
		movingDown  = false;
		movingLeft  = false;

		// if (getSide() == Side::Client)
		// 	getGame().toClient().client->send(StopPlayerMovementPacket());
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

		// if (getSide() == Side::Client)
		// 	getGame().toClient().client->send(StopPlayerMovementPacket(direction));
	}

	void Player::movedToNewChunk(const std::optional<ChunkPosition> &old_position) {
		if (getSide() == Side::Client) {
			if (auto realm = weakRealm.lock()) {
				std::set<ChunkPosition> chunk_requests;
				std::vector<EntityRequest> entity_requests;
				std::vector<TileEntityRequest> tile_entity_requests;

				auto process_chunk = [&](ChunkPosition chunk_position) {
					chunk_requests.insert(chunk_position);

					if (auto entities = realm->getEntities(chunk_position)) {
						auto lock = entities->sharedLock();
						for (const auto &entity: *entities)
							entity_requests.emplace_back(*entity);
					}

					if (auto tile_entities = realm->getTileEntities(chunk_position)) {
						auto lock = tile_entities->sharedLock();
						for (const auto &tile_entity: *tile_entities)
							tile_entity_requests.emplace_back(*tile_entity);
					}
				};

				if (old_position) {
					const ChunkRange old_range(*old_position);
					ChunkRange(getChunk()).iterate([&process_chunk, old_range](ChunkPosition chunk_position) {
						if (!old_range.contains(chunk_position))
							process_chunk(chunk_position);
					});
				} else {
					ChunkRange(getChunk()).iterate(process_chunk);
				}

				if (!chunk_requests.empty())
					send(ChunkRequestPacket(*realm, chunk_requests));

				if (!entity_requests.empty())
					send(EntityRequestPacket(realm->id, std::move(entity_requests)));

				if (!tile_entity_requests.empty())
					send(TileEntityRequestPacket(realm->id, std::move(tile_entity_requests)));
			}

			Entity::movedToNewChunk(old_position);
		} else {
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
						for (const auto &entity: *entities)
							if (!entity->hasBeenSentTo(shared))
								entity->sendTo(client);
					}
				}

				Entity::movedToNewChunk(old_position);

				realm->recalculateVisibleChunks();
			} else {
				Entity::movedToNewChunk(old_position);
			}
		}

	}

	bool Player::send(const Packet &packet) {
		if (getSide() == Side::Server) {
			if (auto locked = toServer()->weakClient.lock()) {
				locked->send(packet);
				return true;
			}
		} else {
			getGame().toClient().client->send(packet);
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
		send(SetPlayerStationTypesPacket(stationTypes, true));
	}

	void Player::removeStationType(const Identifier &station_type) {
		{
			auto lock = stationTypes.uniqueLock();
			if (auto iter = stationTypes.find(station_type); iter != stationTypes.end())
				stationTypes.erase(station_type);
			else
				return;
		}
		send(SetPlayerStationTypesPacket(stationTypes, false));
	}

	PlayerPtr Player::getShared() {
		return std::dynamic_pointer_cast<Player>(shared_from_this());
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
		send(RealmNoticePacket(realm));
		addKnownRealm(realm.id);
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
