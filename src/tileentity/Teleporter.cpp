#include <iostream>

#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "graphics/SpriteRenderer.h"
#include "realm/Realm.h"
#include "tileentity/Teleporter.h"

namespace Game3 {
	Teleporter::Teleporter(Identifier tilename, Position position_, RealmID target_realm, Position target_position):
		TileEntity(std::move(tilename), ID(), position_, false),
		targetRealm(target_realm),
		targetPosition(target_position) {}

	void Teleporter::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["targetRealm"] = targetRealm;
		json["targetPosition"] = targetPosition;
	}

	void Teleporter::onOverlap(const std::shared_ptr<Entity> &entity) {
		if (getSide() != Side::Server)
			return;

		getRealm()->queue([entity, position = targetPosition, target = getRealm()->getGame().getRealm(targetRealm)] {
			entity->teleport(position, target, MovementContext{.isTeleport = true});
		});
	}

	void Teleporter::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		targetRealm = json.at("targetRealm");
		targetPosition = json.at("targetPosition");
	}

	void Teleporter::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << targetRealm;
		buffer << targetPosition;
	}

	void Teleporter::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		buffer >> targetRealm;
		buffer >> targetPosition;
	}
}
