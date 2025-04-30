#include "entity/Player.h"
#include "game/Game.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "lib/JSON.h"
#include "realm/Realm.h"
#include "tileentity/Teleporter.h"

namespace Game3 {
	Teleporter::Teleporter(Identifier tilename, Position position, RealmID targetRealm, Position targetPosition, Identifier soundSetID):
		TileEntity(std::move(tilename), ID(), position, false),
		HasSoundSet(std::move(soundSetID)),
		targetRealm(targetRealm),
		targetPosition(targetPosition) {}

	GamePtr Teleporter::getGame() const {
		return TileEntity::getGame();
	}

	void Teleporter::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		auto &object = ensureObject(json);
		object["targetRealm"] = targetRealm;
		object["targetPosition"] = boost::json::value_from(targetPosition);
		object["soundSetID"] = boost::json::value_from(soundSetID);
	}

	void Teleporter::onOverlap(const std::shared_ptr<Entity> &entity) {
		GamePtr game = getGame();

		if (game->getSide() != Side::Server) {
			return;
		}

		playSound(entity->getPlace());

		getRealm()->queue([entity, position = targetPosition, target = game->getRealm(targetRealm)] {
			entity->teleport(position, target, MovementContext{.isTeleport = true});
		});
	}

	void Teleporter::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		targetRealm = boost::json::value_to<RealmID>(json.at("targetRealm"));
		targetPosition = boost::json::value_to<Position>(json.at("targetPosition"));
		soundSetID = boost::json::value_to<Identifier>(json.at("soundSetID"));
		soundSet.reset();
	}

	void Teleporter::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << targetRealm;
		buffer << targetPosition;
		encodeSoundSet(buffer);
	}

	void Teleporter::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		buffer >> targetRealm;
		buffer >> targetPosition;
		decodeSoundSet(buffer);
	}
}
