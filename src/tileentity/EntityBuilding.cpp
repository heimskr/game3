#include <iostream>

#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "graphics/SpriteRenderer.h"
#include "realm/Realm.h"
#include "tileentity/EntityBuilding.h"

namespace Game3 {
	EntityBuilding::EntityBuilding(Identifier tilename, Position position_, GlobalID target_entity):
		TileEntity(std::move(tilename), ID(), position_, false),
		targetEntity(target_entity) {}

	void EntityBuilding::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["targetEntity"] = targetEntity;
	}

	bool EntityBuilding::onInteractOn(const PlayerPtr &player, Modifiers, const ItemStackPtr &, Hand) {
		teleport(player);
		return true;
	}

	bool EntityBuilding::onInteractNextTo(const PlayerPtr &player, Modifiers, const ItemStackPtr &, Hand) {
		teleport(player);
		return true;
	}

	void EntityBuilding::teleport(const std::shared_ptr<Entity> &entity) {
		GamePtr game = getGame();

		if (game->getSide() != Side::Server)
			return;

		if (!entity) {
			INFO_("No entity");
			return;
		}

		EntityPtr target = game->getAgent<Entity>(targetEntity);
		if (!target) {
			INFO("No target ({})", targetEntity);
			return;
		}

		INFO("Teleporting to {}", targetEntity);
		entity->teleport(target->getPosition(), target->getRealm(), MovementContext{.isTeleport = true});
	}

	void EntityBuilding::absorbJSON(const GamePtr &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		targetEntity = json.at("targetEntity");
	}

	void EntityBuilding::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << targetEntity;
	}

	void EntityBuilding::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		buffer >> targetEntity;
	}
}
