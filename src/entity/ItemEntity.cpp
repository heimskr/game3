#include <nanogui/opengl.h>

#include <iostream>

#include "entity/ItemEntity.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	std::unordered_map<ItemID, Texture> ItemEntity::itemTextureMap {
		{Item::SHORTSWORD, Texture("resources/items/SwordShort.png")},
	};

	Texture ItemEntity::missing = {"resources/missing.png"};

	ItemEntity::ItemEntity(const ItemStack &stack_):
		Entity(Entity::ITEM), stack(stack_) {}

	std::shared_ptr<ItemEntity> ItemEntity::create(const ItemStack &stack) {
		auto out = std::shared_ptr<ItemEntity>(new ItemEntity(stack));
		out->init();
		return out;
	}

	std::shared_ptr<ItemEntity> ItemEntity::fromJSON(const nlohmann::json &json) {
		auto out = create(json.at("stack"));
		out->absorbJSON(json);
		return out;
	}

	void ItemEntity::init() {
		Entity::init();
		texture = &itemTextureMap.at(stack.item->id);
	}

	void ItemEntity::setStack(const ItemStack &stack_) {
		stack   = stack_;
		texture = &itemTextureMap.at(stack_.item->id);
	}

	void ItemEntity::render(SpriteRenderer &sprite_renderer) const {
		if (texture == nullptr)
			return;

		const float x = position.column + offset.x();
		const float y = position.row + offset.y();

		switch (stack.item->id) {
			case Item::SHORTSWORD:
				sprite_renderer.drawOnMap(*texture, x, y, 0.f, 0.f, 16.f, 16.f);
				break;
			default:
				sprite_renderer.drawOnMap(missing, x, y, 0.f, 0.f, 16.f, 16.f);
		}
	}

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
