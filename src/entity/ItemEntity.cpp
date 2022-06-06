#include "entity/ItemEntity.h"

namespace Game3 {
	ItemEntity::ItemEntity(const ItemStack &stack_):
		Entity(Entity::ITEM), stack(stack_) {}

	nlohmann::json ItemEntity::toJSON() const {
		nlohmann::json json;
		to_json(json, *this);
		return json;
	}

	void to_json(nlohmann::json &json, const ItemEntity &item_entity) {
		to_json(json, static_cast<const Entity &>(item_entity));
		json["stack"] = item_entity.stack;
	}

	void from_json(const nlohmann::json &json, ItemEntity &item_entity) {
		from_json(json, static_cast<Entity &>(item_entity));
		item_entity.stack = json.at("stack");
	}
}
