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
#include "packet/EntitySetPathPacket.h"
#include "packet/RealmNoticePacket.h"
#include "packet/StartPlayerMovementPacket.h"
#include "packet/StopPlayerMovementPacket.h"
#include "realm/Realm.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/tab/TextTab.h"

namespace Game3 {
	Player::Player():
		Entity(ID()) {}

	Player::~Player() {
		INFO("~Player(" << this << ')');
	}

	void Player::destroy() {
		auto lock = lockVisibleEntitiesShared();

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
			auto ent_lock = getRealm()->lockEntitiesShared();
			for (const auto &entity: getRealm()->entities) {
				auto vis_lock = entity->lockVisibleEntitiesShared();
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
		if (0.f < tooldown)
			json["tooldown"] = tooldown;
	}

	void Player::absorbJSON(Game &game, const nlohmann::json &json) {
		Entity::absorbJSON(game, json);
		tooldown = json.contains("tooldown")? json.at("tooldown").get<float>() : 0.f;
	}

	void Player::tick(Game &game, float delta) {
		Entity::tick(game, delta);

		if (0.f < tooldown) {
			tooldown -= delta;
			if (tooldown < 0.f) {
				tooldown = 0;
				inventory->notifyOwner();
			}
		}

		if (getSide() == Side::Server) {
			Direction final_direction = direction;

			if (movingLeft && !movingRight)
				move(final_direction = Direction::Left);

			if (movingRight && !movingLeft)
				move(final_direction = Direction::Right);

			if (movingUp && !movingDown)
				move(final_direction = Direction::Up);

			if (movingDown && !movingUp)
				move(final_direction = Direction::Down);

			if (continuousInteraction) {
				Place place = getPlace();
				if (!lastContinuousInteraction || *lastContinuousInteraction != place) {
					interactOn();
					getRealm()->interactGround(getShared(), position, continuousInteractionModifiers);
					lastContinuousInteraction = std::move(place);
				}
			} else {
				lastContinuousInteraction.reset();
			}

			direction = final_direction;
		}
	}

	bool Player::interactOn() {
		auto realm = getRealm();
		auto player = getShared();
		auto entity = realm->findEntity(position, player);
		if (!entity)
			return false;
		return entity->onInteractOn(player);
	}

	void Player::interactNextTo(Modifiers modifiers) {
		auto realm = getRealm();
		const Position next_to = nextTo();
		auto player = getShared();
		auto entity = realm->findEntity(next_to, player);
		bool interesting = false;
		if (entity)
			interesting = entity->onInteractNextTo(player);
		if (!interesting)
			if (auto tileEntity = realm->tileEntityAt(next_to))
				interesting = tileEntity->onInteractNextTo(player);
		if (!interesting)
			realm->interactGround(player, next_to, modifiers);
	}

	void Player::teleport(const Position &position, const std::shared_ptr<Realm> &new_realm) {
		auto &game = new_realm->getGame();
		if (game.activeRealm != new_realm && getSide() == Side::Server)
			send(RealmNoticePacket(*new_realm));
		Entity::teleport(position, new_realm);
		if (game.activeRealm->id != nextRealm && nextRealm != -1) {
			game.activeRealm->onBlur();
			game.activeRealm->queuePlayerRemoval(getShared());
			game.activeRealm = new_realm;
			if (getSide() == Side::Client) {
				game.activeRealm->onFocus();
				new_realm->reupload();
				focus(game.toClient().canvas, true);
			} else {
				auto locked = toServer()->client.lock();
				assert(locked);
				INFO("Sending " << new_realm->id << " to client");
				new_realm->sendTo(*locked);
			}
		}
	}

	void Player::addMoney(MoneyCount to_add) {
		money += to_add;
		auto &game = getRealm()->getGame();
		if (game.getSide() == Side::Client)
			game.toClient().signal_player_money_update().emit(getShared());
	}

	bool Player::setTooldown(float multiplier) {
		if (auto *active = inventory->getActive())
			if (auto tool = std::dynamic_pointer_cast<Tool>(active->item)) {
				tooldown = multiplier * tool->baseCooldown;
				return true;
			}
		return false;
	}

	void Player::showText(const Glib::ustring &text, const Glib::ustring &name) {
		if (getSide() == Side::Client) {
			getRealm()->getGame().toClient().setText(text, name, true, true);
			queueForMove([player = shared_from_this()](const auto &) {
				player->getRealm()->getGame().toClient().canvas.window.textTab->hide();
				return true;
			});
		}
	}

	void Player::give(const ItemStack &stack, Slot start) {
		if (auto leftover = inventory->add(stack, start))
			getRealm()->spawn<ItemEntity>(getPosition(), *leftover);
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
		weakRealm = game.realms.at(realmID);
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
		}

		if (getSide() == Side::Client)
			getGame().toClient().client->send(StartPlayerMovementPacket(direction));
	}

	void Player::stopMoving() {
		movingUp    = false;
		movingRight = false;
		movingDown  = false;
		movingLeft  = false;

		if (getSide() == Side::Client)
			getGame().toClient().client->send(StopPlayerMovementPacket());
	}

	void Player::stopMoving(Direction direction) {
		switch (direction) {
			case Direction::Up:    movingUp    = false; break;
			case Direction::Right: movingRight = false; break;
			case Direction::Down:  movingDown  = false; break;
			case Direction::Left:  movingLeft  = false; break;
		}

		if (getSide() == Side::Client)
			getGame().toClient().client->send(StopPlayerMovementPacket(direction));
	}

	void Player::movedToNewChunk() {
		if (getSide() != Side::Server)
			return;

		{
			auto shared = getShared();
			auto lock = lockVisibleEntitiesShared();
			for (const auto &weak_visible: visibleEntities) {
				if (auto visible = weak_visible.lock()) {
					if (!visible->path.empty() && visible->hasSeenPath(shared)) {
						// INFO("Late sending EntitySetPathPacket (Player)");
						toServer()->ensureEntity(visible);
						send(EntitySetPathPacket(*visible));
						visible->setSeenPath(shared);
					}

					if (!canSee(*visible)) {
						auto visible_lock = visible->lockVisibleEntities();
						visible->visiblePlayers.erase(shared);
						visible->visibleEntities.erase(shared);
					}
				}
			}
		}

		Entity::movedToNewChunk();

		getRealm()->recalculateVisibleChunks();
	}

	bool Player::send(const Packet &packet) {
		if (getSide() == Side::Server) {
			if (auto locked = toServer()->client.lock()) {
				locked->send(packet);
				return true;
			}
		} else {
			getGame().toClient().client->send(packet);
			return true;
		}

		return false;
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
