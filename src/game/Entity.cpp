#include <nanogui/opengl.h>

#include <iostream>

#include "game/Entity.h"
#include "game/Game.h"
#include "game/Realm.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	std::unordered_map<EntityID, Texture> Entity::textureMap {
		{Entity::GANGBLANC, Texture("resources/characters/champions/Gangblanc.png")},
	};

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

		sprite_renderer.draw(*texture, position.second, position.first, 0.f, 8.f * int(direction), 16.f, 16.f);
	}

	void Entity::move(Direction move_direction) {
		auto realm = weakRealm.lock();
		if (!realm)
			return;

		switch (move_direction) {
			case Direction::Down:
				if (int(position.first) < realm->tilemap1->height - 1)
					++position.first;
				direction = Direction::Down;
				break;
			case Direction::Up:
				if (position.first != 0)
					--position.first;
				direction = Direction::Up;
				break;
			case Direction::Left:
				if (position.second != 0)
					--position.second;
				direction = Direction::Left;
				break;
			case Direction::Right:
				if (int(position.second) < realm->tilemap1->width - 1)
					++position.second;
				direction = Direction::Right;
				break;
			default:
				throw std::invalid_argument("Invalid direction: " + std::to_string(int(move_direction)));
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
