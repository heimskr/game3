#include <nanogui/opengl.h>

#include <iostream>

#include "game/Entity.h"
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

		sprite_renderer.draw(*texture, position.first, position.second, 0.f, 0.f, 16.f, 16.f);
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
