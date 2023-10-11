#include "Log.h"
#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "graphics/ItemTexture.h"
#include "graphics/SpriteRenderer.h"
#include "item/Item.h"
#include "net/Buffer.h"
#include "realm/Realm.h"
#include "registry/Registries.h"
#include "ui/Canvas.h"

namespace Game3 {
	ItemEntity::ItemEntity(const Game &game, const nlohmann::json &):
		Entity(ID()), stack(game) {}

	ItemEntity::ItemEntity(ItemStack stack_):
		Entity(ID()), stack(std::move(stack_)) {}

	void ItemEntity::setStack(ItemStack stack_) {
		stack = std::move(stack_);
		setTexture(getRealm()->getGame());
	}

	void ItemEntity::setTexture(const Game &game) {
		if (getSide() != Side::Client)
			return;
		std::shared_ptr<ItemTexture> item_texture = game.registry<ItemTextureRegistry>().at(stack.item->identifier);
		texture = stack.getTexture(game);
		texture->init();
		xOffset = item_texture->x / 2.f;
		yOffset = item_texture->y / 2.f;
		sizeX = float(item_texture->width);
		sizeY = float(item_texture->height);
	}

	std::shared_ptr<ItemEntity> ItemEntity::create(Game &) {
		return Entity::create<ItemEntity>();
	}

	std::shared_ptr<ItemEntity> ItemEntity::create(Game &, const ItemStack &stack) {
		return Entity::create<ItemEntity>(stack);
	}

	std::shared_ptr<ItemEntity> ItemEntity::fromJSON(Game &game, const nlohmann::json &json) {
		if (json.is_null())
			return create(game, ItemStack(game));
		auto out = create(game, ItemStack::fromJSON(game, json.at("stack")));
		out->absorbJSON(game, json);
		return out;
	}

	void ItemEntity::toJSON(nlohmann::json &json) const {
		Entity::toJSON(json);
		json["stack"] = getStack();
	}

	void ItemEntity::init(Game &game) {
		Entity::init(game);
		if (stack.item && getSide() == Side::Client)
			stack.item->getOffsets(game, texture, xOffset, yOffset);
	}

	void ItemEntity::tick(Game &, float delta) {
		secondsLeft = secondsLeft - delta;
		if (secondsLeft <= 0)
			queueDestruction();
	}

	void ItemEntity::render(SpriteRenderer &sprite_renderer, TextRenderer &) {
		if (!isVisible())
			return;

		if (texture == nullptr || needsTexture) {
			if (needsTexture) {
				setTexture(*sprite_renderer.canvas->game);
				needsTexture = false;
			} else
				return;
		}

		const float x = position.column + offset.x;
		const float y = position.row + offset.y;

		sprite_renderer(*texture, {
			.x = x + .125f,
			.y = y + .125f,
			.xOffset = xOffset,
			.yOffset = yOffset,
			.sizeX = sizeX,
			.sizeY = sizeY,
			.scaleX = .75f * 16.f / sizeX,
			.scaleY = .75f * 16.f / sizeY,
		});
	}

	bool ItemEntity::interact(const std::shared_ptr<Player> &player) {
		if (getSide() != Side::Server)
			return true;

		auto leftover = player->getInventory()->add(stack);
		if (leftover) {
			stack = std::move(*leftover);
			increaseUpdateCounter();
		} else
			remove();

		return true;
	}

	std::string ItemEntity::getName() const {
		return stack.item->name;
	}

	void ItemEntity::encode(Buffer &buffer) {
		Entity::encode(buffer);
		stack.encode(getGame(), buffer);
		buffer << secondsLeft;
	}

	void ItemEntity::decode(Buffer &buffer) {
		Entity::decode(buffer);
		stack.decode(getGame(), buffer);
		buffer >> secondsLeft;
	}

	void to_json(nlohmann::json &json, const ItemEntity &item_entity) {
		item_entity.toJSON(json);
	}
}
