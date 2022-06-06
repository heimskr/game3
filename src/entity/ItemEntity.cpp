#include <iostream>

#include "entity/ItemEntity.h"

namespace Game3 {
	std::unordered_map<EntityID, Texture> ItemEntity::itemTextureMap {
		{Item::SHORTSWORD, Texture("resources/items/ShortSword.png")},
	};

	ItemEntity::ItemEntity(const ItemStack &stack_):
		Entity(Entity::ITEM), stack(stack_) {}

	std::shared_ptr<ItemEntity> ItemEntity::create(const ItemStack &stack) {
		return std::shared_ptr<ItemEntity>(new ItemEntity(stack));
	}

	std::shared_ptr<ItemEntity> ItemEntity::fromJSON(const nlohmann::json &json) {
		return create(json.at("stack"));
	}

	void ItemEntity::setStack(const ItemStack &stack_) {
		stack   = stack_;
		texture = &itemTextureMap.at(stack_.item->id);
	}

	// void ItemEntity::render(SpriteRenderer &sprite_renderer) const {
	// 	if (texture == nullptr)
	// 		return;
		
	// 	std::cout << "ItemEntity::render\n";
	// }

	nlohmann::json ItemEntity::toJSON() const {
		nlohmann::json json;
		to_json(json, *this);
		return json;
	}

	void to_json(nlohmann::json &json, const ItemEntity &item_entity) {
		to_json(json, static_cast<const Entity &>(item_entity));
		json["stack"] = item_entity.getStack();
	}
}
