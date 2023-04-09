#include <iostream>

#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "realm/Realm.h"
#include "registry/Registries.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	Texture ItemEntity::missing = {"resources/missing.png"};

	ItemEntity::ItemEntity(const ItemStack &stack_):
		Entity(Entity::ITEM_ID, Entity::ITEM_TYPE), stack(stack_) {}

	void ItemEntity::setStack(const ItemStack &stack_) {
		stack = stack_;
		const Game &game = getRealm()->getGame();
		auto item_texture = game.registry<ItemTextureRegistry>().at(stack_.item->id);
		texture = item_texture->texture.lock().get();
		xOffset = item_texture->x / 2.f;
		yOffset = item_texture->y / 2.f;
	}

	std::shared_ptr<ItemEntity> ItemEntity::create(Game &game, const ItemStack &stack) {
		auto out = std::shared_ptr<ItemEntity>(std::unique_ptr<ItemEntity>(new ItemEntity(stack)));
		out->init(game);
		return out;
	}

	std::shared_ptr<ItemEntity> ItemEntity::fromJSON(Game &game, const nlohmann::json &json) {
		ItemStack stack;
		ItemStack::fromJSON(game, json.at("stack"), stack);
		auto out = create(game, stack);
		out->absorbJSON(json);
		return out;
	}

	void ItemEntity::toJSON(nlohmann::json &json) const {
		Entity::toJSON(json);
		json["stack"] = getStack();
	}

	void ItemEntity::init(Game &game) {
		Entity::init(game);
		stack.item->getOffsets(game, texture, xOffset, yOffset);
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
