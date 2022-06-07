#include <iostream>

#include "entity/Player.h"
#include "game/Game.h"
#include "game/Realm.h"

namespace Game3 {
	std::shared_ptr<Player> Player::fromJSON(const nlohmann::json &json) {
		auto out = Entity::create<Player>(json.at("id"));
		out->absorbJSON(json);
		return out;
	}

	nlohmann::json Player::toJSON() const {
		nlohmann::json json;
		to_json(json, *this);
		return json;
	}

	void Player::interactOn() {
		auto realm = getRealm();
		auto player = std::dynamic_pointer_cast<Player>(shared_from_this());
		auto entity = realm->findEntity(position, player);
		if (!entity)
			return;
		entity->onInteractOn(player);
	}

	void Player::interactNextTo() {
		auto realm = getRealm();
		auto player = std::dynamic_pointer_cast<Player>(shared_from_this());
		auto entity = realm->findEntity(nextTo(), player);
		if (entity)
			entity->onInteractNextTo(player);
		else if (auto tileEntity = realm->tileEntityAt(nextTo()))
			tileEntity->onInteractNextTo(player);
	}

	void Player::teleport(const Position &position, const std::shared_ptr<Realm> &new_realm) {
		if (!new_realm->game)
			throw std::runtime_error("Can't teleport player to realm " + std::to_string(new_realm->id) + ": game is null");
		Entity::teleport(position, new_realm);
		new_realm->reupload();
		new_realm->game->activeRealm = new_realm;
		focus(new_realm->game->canvas);
	}

	void to_json(nlohmann::json &json, const Player &player) {
		to_json(json, static_cast<const Entity &>(player));
		json["isPlayer"] = true;
	}
}
