#include "game/Entity.h"

namespace Game3 {
	std::unordered_map<EntityID, std::string> Entity::textureMap {
		{Entity::GANGBLANC, "resources/characters/champions/Gangblanc.png"}
	};

	void to_json(nlohmann::json &json, const Entity &entity) {
		json["id"] = entity.id;
		json["position"] = entity.position;
		json["realmID"] = entity.realmID;
		json["direction"] = entity.direction;
		json["inventory"] = entity.inventory;
	}

	void from_json(const nlohmann::json &json, Entity &entity) {
		entity.id = json.at("id");
		entity.position = json.at("position");
		entity.realmID = json.at("realmID");
		entity.direction = json.at("direction");
		entity.inventory = json.at("inventory");
	}
}
