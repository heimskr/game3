#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "graphics/ItemTexture.h"
#include "graphics/RendererContext.h"
#include "graphics/SpriteRenderer.h"
#include "item/Item.h"
#include "net/Buffer.h"
#include "realm/Realm.h"
#include "registry/Registries.h"
#include "ui/Window.h"
#include "util/Log.h"

namespace Game3 {
	ItemEntity::ItemEntity(const GamePtr &game):
		Entity(ID()), stack(ItemStack::create(game)) {}

	ItemEntity::ItemEntity(ItemStackPtr stack_):
		Entity(ID()), stack(std::move(stack_)) {}

	void ItemEntity::setStack(ItemStackPtr stack_) {
		stack = std::move(stack_);
		setTexture(getRealm()->getGame());
	}

	void ItemEntity::setTexture(const GamePtr &game) {
		if (getSide() != Side::Client) {
			return;
		}
		std::shared_ptr<ItemTexture> item_texture = game->registry<ItemTextureRegistry>().at(stack->item->identifier);
		texture = item_texture->getTexture();
		texture->init();
		offsetX = item_texture->x / 2.f;
		offsetY = item_texture->y / 2.f;
		sizeX = float(item_texture->width);
		sizeY = float(item_texture->height);
	}

	std::shared_ptr<ItemEntity> ItemEntity::create(const GamePtr &) {
		return Entity::create<ItemEntity>();
	}

	std::shared_ptr<ItemEntity> ItemEntity::create(const GamePtr &, ItemStackPtr stack) {
		return Entity::create<ItemEntity>(std::move(stack));
	}

	std::shared_ptr<ItemEntity> ItemEntity::fromJSON(const GamePtr &game, const boost::json::value &json) {
		if (json.is_null()) {
			return create(game, ItemStack::create(game));
		}
		auto out = create(game, boost::json::value_to<ItemStackPtr>(json.at("stack"), game));
		out->absorbJSON(game, json);
		return out;
	}

	void ItemEntity::toJSON(boost::json::value &json) const {
		Entity::toJSON(json);
		json.as_object()["stack"] = boost::json::value_from(*getStack());
	}

	void ItemEntity::init(const GamePtr &game) {
		Entity::init(game);
		if (!stack) {
			stack = ItemStack::create(game);
		} else if (stack->item && getSide() == Side::Client) {
			stack->item->getOffsets(*game, texture, offsetX, offsetY);
		}
	}

	void ItemEntity::tick(const TickArgs &args) {
		if (firstTick) {
			firstTick = false;
		} else {
			age += args.delta;
		}

		if (age >= secondsLeft) {
			remove();
		} else {
			applyMotion(args.delta);
			enqueueTick();
		}
	}

	void ItemEntity::render(const RendererContext &renderers) {
		SpriteRenderer &sprite_renderer = renderers.batchSprite;

		if (!isVisible()) {
			return;
		}

		if (texture == nullptr || needsTexture) {
			if (needsTexture) {
				setTexture(sprite_renderer.window->game);
				needsTexture = false;
			} else {
				return;
			}
		}

		const float x = position.column + offset.x;
		const float y = position.row + offset.y - offset.z;

		sprite_renderer(texture, {
			.x = x + .125f,
			.y = y + .125f,
			.offsetX = offsetX,
			.offsetY = offsetY,
			.sizeX = sizeX,
			.sizeY = sizeY,
			.scaleX = .75f * 16.f / sizeX,
			.scaleY = .75f * 16.f / sizeY,
		});
	}

	bool ItemEntity::interact(const std::shared_ptr<Player> &player) {
		if (getSide() != Side::Server) {
			return true;
		}

		if (ItemStackPtr leftover = player->getInventory(0)->add(stack)) {
			stack = std::move(leftover);
			increaseUpdateCounter();
		} else {
			remove();
		}

		return true;
	}

	std::string ItemEntity::getName() const {
		return stack->item->name;
	}

	void ItemEntity::encode(Buffer &buffer) {
		Entity::encode(buffer);
		GamePtr game = getGame();
		if (!stack) {
			stack = ItemStack::create(game);
		}
		stack->encode(*game, buffer);
		buffer << secondsLeft;
	}

	void ItemEntity::decode(Buffer &buffer) {
		Entity::decode(buffer);
		GamePtr game = getGame();
		if (!stack) {
			stack = ItemStack::create(game);
		}
		stack->decode(*game, buffer);
		buffer >> secondsLeft;
	}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const ItemEntity &item_entity) {
		item_entity.toJSON(json);
	}
}
