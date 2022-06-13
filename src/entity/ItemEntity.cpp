#include <iostream>

#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	Texture ItemEntity::missing = {"resources/missing.png"};

	ItemEntity::ItemEntity(const ItemStack &stack_):
		Entity(Entity::ITEM), stack(stack_) {}

	void ItemEntity::setStack(const ItemStack &stack_) {
		stack = stack_;
		const auto &item_texture = Item::itemTextures.at(stack_.item->id);
		texture = item_texture.texture;
		xOffset = item_texture.x / 2.f;
		yOffset = item_texture.y / 2.f;
	}

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

	nlohmann::json ItemEntity::toJSON() const {
		nlohmann::json json;
		to_json(json, *this);
		return json;
	}

	void ItemEntity::init() {
		Entity::init();
		const auto &item_texture = Item::itemTextures.at(stack.item->id);
		texture = item_texture.texture;
		xOffset = item_texture.x / 2.f;
		yOffset = item_texture.y / 2.f;
	}

	void ItemEntity::render(SpriteRenderer &sprite_renderer) const {
		if (texture == nullptr)
			return;

		const float x = position.column + offset.x();
		const float y = position.row + offset.y();

		switch (stack.item->id) {
			case Item::SHORTSWORD:
				sprite_renderer.drawOnMap(*texture, x, y, xOffset, yOffset, 16.f, 16.f);
				break;
			case Item::RED_POTION:
				sprite_renderer.drawOnMap(*texture, x + .25f, y + .25f, xOffset, yOffset, 16.f, 16.f, .5f);
				break;
			default:
				sprite_renderer.drawOnMap(missing, x, y, 0.f, 0.f, 16.f, 16.f);
		}
	}

	void ItemEntity::interact(const std::shared_ptr<Player> &player) {
		auto leftover = player->inventory->add(stack);
		if (leftover)
			stack = std::move(*leftover);
		else
			remove();
	}

	void to_json(nlohmann::json &json, const ItemEntity &item_entity) {
		to_json(json, static_cast<const Entity &>(item_entity));
		json["stack"] = item_entity.getStack();
	}
}
