#include <nanogui/opengl.h>

#include <iostream>

#include "Tiles.h"
#include "game/Entity.h"
#include "game/Game.h"
#include "game/Realm.h"
#include "ui/Canvas.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	std::unordered_map<EntityID, Texture> Entity::textureMap {
		{Entity::GANGBLANC, Texture("resources/characters/champions/Gangblanc.png")},
	};

	std::shared_ptr<Entity> Entity::fromJSON(const nlohmann::json &json) {
		if (json.at("isPlayer") == true)
			return std::make_shared<Player>(json);
		return std::make_shared<Entity>(json);
	}

	nlohmann::json Entity::toJSON() const {
		nlohmann::json json;
		to_json(json, *this);
		return json;
	}

	void Entity::tick(float delta) {
		auto &x = offset.x();
		auto &y = offset.y();
		if (x < 0.f)
			x = std::min(x + delta * speed, 0.f);
		else if (0.f < x)
			x = std::max(x - delta * speed, 0.f);
		if (y < 0.f)
			y = std::min(y + delta * speed, 0.f);
		else if (0.f < y)
			y = std::max(y - delta * speed, 0.f);
	}

	void Entity::id(EntityID new_id) {
		id_ = new_id;
		texture = &textureMap.at(id_);
	}

	void Entity::init() {
		texture = &textureMap.at(id_);
	}

	void Entity::render(SpriteRenderer &sprite_renderer) const {
		if (!texture)
			return;

		sprite_renderer.drawOnMap(*texture, position.second + offset.x(), position.first + offset.y(), 0.f, 8.f * int(direction), 16.f, 16.f);
	}

	void Entity::move(Direction move_direction) {
		auto realm = weakRealm.lock();
		if (!realm)
			return;

		Position new_position = position;
		float x_offset = 0.f;
		float y_offset = 0.f;
		switch (move_direction) {
			case Direction::Down:
				++new_position.first;
				direction = Direction::Down;
				y_offset = -1.f;
				break;
			case Direction::Up:
				--new_position.first;
				direction = Direction::Up;
				y_offset = 1.f;
				break;
			case Direction::Left:
				--new_position.second;
				direction = Direction::Left;
				x_offset = 1.f;
				break;
			case Direction::Right:
				++new_position.second;
				direction = Direction::Right;
				x_offset = -1.f;
				break;
			default:
				throw std::invalid_argument("Invalid direction: " + std::to_string(int(move_direction)));
		}

		if (offset.x() != 0 || offset.y() != 0)
			return;

		if (canMoveTo(new_position)) {
			position = new_position;
			offset = {x_offset, y_offset};
		}
	}

	void Entity::setRealm(const Game &game, RealmID realm_id) {
		weakRealm = game.realms.at(realm_id);
		realmID = realm_id;
	}

	void Entity::setRealm(const std::shared_ptr<Realm> realm) {
		weakRealm = realm;
		realmID = realm->id;
	}

	bool Entity::canMoveTo(const Position &new_position) const {
		if (new_position.first < 0 || new_position.second < 0)
			return false;

		auto realm = weakRealm.lock();
		if (!realm)
			return false;

		if (realm->getHeight() <= new_position.first || realm->getWidth() <= new_position.second)
			return false;

		if (!isLand((*realm->tilemap1)(new_position.second, new_position.first)))
			return false;

		if (isSolid((*realm->tilemap2)(new_position.second, new_position.first)))
			return false;

		return true;
	}

	void Entity::focus(Canvas &canvas) {
		auto realm = weakRealm.lock();
		if (!realm)
			return;

		const auto &tilemap = *realm->tilemap1;
		canvas.scale = 4.f;
		canvas.center.x() = -(column() - tilemap.width  / 2.f);
		canvas.center.y() = -(row()    - tilemap.height / 2.f);
	}

	void to_json(nlohmann::json &json, const Entity &entity) {
		json["id"] = entity.id();
		json["position"] = entity.position;
		json["realmID"] = entity.realmID;
		json["direction"] = entity.direction;
		json["inventory"] = entity.inventory;
	}

	void from_json(const nlohmann::json &json, Entity &entity) {
		entity.id(json.at("id"));
		entity.position = json.at("position");
		entity.realmID = json.at("realmID");
		entity.direction = json.at("direction");
		entity.inventory = json.at("inventory");
	}
}
