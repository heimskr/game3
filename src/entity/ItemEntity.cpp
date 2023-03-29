#include <iostream>

#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	Texture ItemEntity::missing = {"resources/missing.png"};

	ItemEntity::ItemEntity(const ItemStack &stack_):
		Entity(Entity::ITEM_ID, Entity::ITEM_TYPE), stack(stack_) {}

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

	void ItemEntity::toJSON(nlohmann::json &json) const {
		Entity::toJSON(json);
		json["stack"] = getStack();
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

		sprite_renderer.drawOnMap(*texture, x + .125f, y + .125f, xOffset, yOffset, 16.f, 16.f, .75f);
	}

	bool ItemEntity::interact(const std::shared_ptr<Player> &player) {
		auto leftover = player->inventory->add(stack);
		if (leftover)
			stack = std::move(*leftover);
		else
			remove();
		return true;
	}

	Glib::ustring ItemEntity::getName() {
		return stack.item->name;
	}

	void to_json(nlohmann::json &json, const ItemEntity &item_entity) {
		item_entity.toJSON(json);
	}
}
