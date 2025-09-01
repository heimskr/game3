#include "entity/Player.h"
#include "game/Game.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "lib/JSON.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "tileentity/Building.h"

namespace Game3 {
	Building::Building(Identifier id, Position position, RealmID innerRealmID, Position entrance, Identifier soundSetID):
		TileEntity(std::move(id), ID(), position, true),
		HasSoundSet(std::move(soundSetID)),
		innerRealmID(innerRealmID),
		entrance(entrance) {}

	GamePtr Building::getGame() const {
		return TileEntity::getGame();
	}

	void Building::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		auto &object = ensureObject(json);
		object["innerRealmID"] = innerRealmID;
		object["entrance"] = boost::json::value_from(entrance);
		object["soundSetID"] = boost::json::value_from(soundSetID);
	}

	bool Building::onInteractOn(const std::shared_ptr<Player> &player, Modifiers modifiers, const ItemStackPtr &used_item, Hand hand) {
		return onInteractNextTo(player, modifiers, used_item, hand);
	}

	bool Building::onInteractNextTo(const std::shared_ptr<Player> &player, Modifiers, const ItemStackPtr &, Hand) {
		if (getSide() == Side::Client) {
			return false;
		}

		playSound(player->getPlace());
		teleport(player);
		return true;
	}

	void Building::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		if (const auto *object = json.if_object()) {
			innerRealmID = boost::json::value_to<RealmID>(object->at("innerRealmID"));
			entrance = boost::json::value_to<Position>(object->at("entrance"));
			if (const auto *value = object->if_contains("soundSetID")) {
				soundSetID = boost::json::value_to<Identifier>(*value);
			}
		}
		soundSet.reset();
	}

	void Building::teleport(const std::shared_ptr<Entity> &entity) {
		entity->teleport(entrance, getInnerRealm());
	}

	std::shared_ptr<Realm> Building::getInnerRealm() const {
		GamePtr game = getGame();
		return game->getRealm(innerRealmID);
	}

	void Building::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << innerRealmID;
		buffer << entrance;
		encodeSoundSet(buffer);
	}

	void Building::decode(Game &game, BasicBuffer &buffer) {
		TileEntity::decode(game, buffer);
		buffer >> innerRealmID;
		buffer >> entrance;
		decodeSoundSet(buffer);
	}
}
